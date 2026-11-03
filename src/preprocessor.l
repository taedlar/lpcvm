%{
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "lpc_compiler.hpp"
#undef yylex
#include "lpc_parser.hpp"
#define yylex pplex

// The preprocessor returns feed status, not LPC parser tokens.
#undef YY_DECL
#define YY_DECL int pplex (yyscan_t yyscanner)
#include "symbol_table.hpp"
#include <algorithm>
#include <cctype>
#include <functional>
#include <string_view>

// Track original input independently of the downstream lexer's buffered input.
#define YY_USER_ACTION \
    yyextra->pp_location.step(); \
    for (int offset = 0; offset < yyleng; ++offset) { \
        if (yytext[offset] == '\n') yyextra->pp_location.lines(1); \
        else yyextra->pp_location.columns(1); \
    }

// append the matched text to the preprocessed output
#define EMIT() yyextra->ppbuffer.insert(yyextra->ppbuffer.end(), yytext, yytext + yyleng)

int preprocessor_feed (char* buf, int max_len, LpcCompiler::Context* ctx);

static bool is_ident_start (char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
static bool is_ident_char (char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

/// Parse "#define NAME body" / "#undef NAME" text: returns the directive operand (after the keyword).
static std::string_view directive_operand (const char* text, int len, std::string_view keyword) {
    std::string_view line(text, len);
    auto pos = line.find(keyword);
    line.remove_prefix(pos + keyword.size());
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.remove_prefix(1);
    return line;
}

static void define_macro (LpcCompiler::Context* ctx, std::string_view operand, yy::location location) {
    // Splice physical lines before parsing, retaining positions for diagnostics.
    std::string logical_operand;
    std::vector<yy::position> positions;
    auto position = location.begin;
    for (size_t i = 0; i < operand.size();) {
        size_t splice = 0;
        if (operand[i] == '\\') { // Check for line continuation
            if (operand.substr(i + 1, 1) == "\n") splice = 2;
            else if (operand.substr(i + 1, 2) == "\r\n") splice = 3;
        }
        if (splice) {
            i += splice;
            position.lines(1);
        } else {
            positions.push_back(position);
            logical_operand.push_back(operand[i++]);
            position.columns(1);
        }
    }
    positions.push_back(position);
    operand = logical_operand;

    size_t operand_start = 0;
    while (!operand.empty() && (operand.front() == ' ' || operand.front() == '\t')) {
        operand.remove_prefix(1);
        ++operand_start;
    }
    auto fail_at = [&](size_t offset, const char* message) {
        auto error_location = location;
        error_location.begin = error_location.end = positions[operand_start + offset];
        throw yy::LpcParser::syntax_error(error_location, message);
    };
    size_t n = 0;

    // Find the end of the macro name
    while (n < operand.size() && is_ident_char(operand[n])) n++;
    if (n == 0 || !is_ident_start(operand[0])) {
        fail_at(0, "Invalid macro name in #define");
    }
    std::string name(operand.substr(0, n));
    std::string_view body = operand.substr(n);

    // Parse function-like macro parameters if present.
    std::vector<std::string> params;
    const bool function_like = !body.empty() && body.front() == '(';
    if (function_like) {
        body.remove_prefix(1); // consume '('
        auto skip_space = [&] {
            while (!body.empty() && std::isspace(static_cast<unsigned char>(body.front())))
                body.remove_prefix(1);
        };
        auto fail = [&](const char* message) {
            fail_at(operand.size() - body.size(), message);
        };
        skip_space();
        if (!body.empty() && body.front() == ')') {
            body.remove_prefix(1); // empty parameter list
        } else {
            while (true) {
                if (body.empty()) fail("Unterminated parameter list in #define");
                if (!is_ident_start(body.front())) fail("Invalid macro parameter in #define");
                size_t end = 1;
                while (end < body.size() && is_ident_char(body[end])) end++;
                params.emplace_back(body.substr(0, end));
                body.remove_prefix(end);
                skip_space();
                if (body.empty()) fail("Unterminated parameter list in #define");
                if (body.front() == ')') {
                    body.remove_prefix(1);
                    break;
                }
                if (body.front() != ',') fail("Expected ',' or ')' in #define parameter list");
                body.remove_prefix(1);
                skip_space();
            }
        }
    }

    // Remove any trailing comments and trim whitespace from the macro body.
    if (auto c = body.find("//"); c != std::string_view::npos)
        body = body.substr(0, c);
    while (!body.empty() && std::isspace(static_cast<unsigned char>(body.front()))) body.remove_prefix(1);
    while (!body.empty() && std::isspace(static_cast<unsigned char>(body.back()))) body.remove_suffix(1);

    SymbolSignature sig{};
    sig.b.is_const = 1;
    if (function_like)
        sig.b.is_function = 1; // function-like macro
    auto entry = std::make_shared<SymbolTable::Entry>(name, sig);
    entry->value = ctx->defines->new_token(body, SymbolTable::TokenType::MACRO_BODY);
    for(int i = params.size() - 1; i >= 0; --i) {
        entry->prepend_token(*ctx->defines, ctx->defines->new_token(params[i], SymbolTable::TokenType::MACRO_ARGUMENT));
    }
    ctx->defines->emplace(name, entry); // entry -> {arg#1}{arg#2}...{body}
}

static void undef_macro (LpcCompiler::Context* ctx, std::string_view operand) {
    size_t n = 0;
    while (n < operand.size() && is_ident_char(operand[n])) n++;
    if (n > 0)
        ctx->defines->erase(operand.substr(0, n));
}

// The same expansion parser reads either a replacement body or the Flex input.
struct ExpansionInput {
    std::function<int()> peek;
    std::function<int()> take;
};

static int scanner_peek (yyscan_t scanner);
static int scanner_take (yyscan_t scanner);
static void expand_text (LpcCompiler::Context* ctx, std::string_view text,
                         std::vector<char>& out, std::vector<std::string>& active);

static void expand_identifier (LpcCompiler::Context* ctx, std::string_view id,
                               ExpansionInput& input, std::vector<char>& out,
                               std::vector<std::string>& active) {
    const SymbolTable* owner = nullptr; // could be predefined macros or user-defined macros
    auto entry = ctx->defines ? ctx->defines->lookup(id, &owner) : nullptr;
    auto macro_def = (entry && entry->value) ? std::get<SymbolTable::TokenIndex>(*entry->value) : SymbolTable::invalid_token;
    if (macro_def == SymbolTable::invalid_token || std::find(active.begin(), active.end(), id) != active.end()) {
        out.insert(out.end(), id.begin(), id.end()); // Insert the identifier as-is if it is not a macro or is currently active.
        return;
    }
    // Collect macro parameters.
    std::vector<std::string_view> params;
    while (macro_def != SymbolTable::invalid_token &&
           owner->token(macro_def).type == SymbolTable::TokenType::MACRO_ARGUMENT) {
        params.push_back(owner->token_text(macro_def));
        macro_def = owner->token(macro_def).next;
    }
    if (macro_def == SymbolTable::invalid_token ||
        owner->token(macro_def).type != SymbolTable::TokenType::MACRO_BODY) {
        out.insert(out.end(), id.begin(), id.end()); // Insert the identifier as-is if macro body is not found.
        return;
    }

    // Do macro expansion for the body.
    std::string body(owner->token_text(macro_def));
    if (entry->signature.b.is_function) {
        const auto location = ctx->pp_location;
        std::string space;
        while (input.peek() && std::isspace(static_cast<unsigned char>(input.peek())))
            space.push_back(static_cast<char>(input.take())); // Collect leading whitespace before the macro argument list.
        if (input.peek() != '(') {
            out.insert(out.end(), id.begin(), id.end()); // Insert the identifier as-is if the macro argument list is not found.
            out.insert(out.end(), space.begin(), space.end()); // Insert the collected leading whitespace.
            return;
        }
        input.take(); // consume '('
        std::vector<std::string> args(1); // actual macro arguments, initially empty.
        int depth = 0; // Track the nesting level of parentheses within the macro argument list.
        int quote = 0; // Track the current quote character ('"' or '\'') if inside a quoted string.
        while (true) {
            int c = input.take();
            if (!c)
                throw yy::LpcParser::syntax_error(location, "Unterminated macro argument list");
            if (quote) { // Inside a quoted string within the macro argument list.
                args.back().push_back(static_cast<char>(c));
                if (c == '\\') {
                    if (int escaped = input.take()) args.back().push_back(static_cast<char>(escaped));
                    else throw yy::LpcParser::syntax_error(location, "Unterminated macro argument list");
                } else if (c == quote) quote = 0;
                continue;
            }
            if (c == '/' && (input.peek() == '/' || input.peek() == '*')) { // Handle comments within macro arguments.
                int kind = input.take();
                int previous = 0;
                while (int comment = input.take()) {
                    if (kind == '/' && comment == '\n') break;
                    if (kind == '*' && previous == '*' && comment == '/') break;
                    previous = comment;
                }
                args.back().push_back(' ');
                continue;
            }
            if (c == '"' || c == '\'') quote = c;
            else if (c == '(') ++depth;
            else if (c == ')') {
                if (depth == 0) break;
                --depth;
            } else if (c == ',' && depth == 0) {
                args.emplace_back();
                continue;
            }
            args.back().push_back(static_cast<char>(c)); // Append the current character to the current macro argument.
        }
        // Trim leading and trailing whitespace from each macro argument.
        for (auto& arg : args) {
            auto first = arg.find_first_not_of(" \t\r\n");
            arg = first == std::string::npos ? "" : arg.substr(first, arg.find_last_not_of(" \t\r\n") - first + 1);
        }
        if (args.size() == 1 && args[0].empty() && params.empty()) args.clear(); // empty argument list for a macro with no parameters.
        if (args.size() != params.size())
            throw yy::LpcParser::syntax_error(location, "Wrong number of macro arguments for " + std::string(id));
        // Expand each macro argument recursively.
        for (auto& arg : args) {
            std::vector<char> expanded;
            expand_text(ctx, arg, expanded, active);
            arg.assign(expanded.begin(), expanded.end());
        }
        // Substitute complete identifiers, leaving string literals and numeric tokens intact.
        std::string replacement;
        for (size_t i = 0; i < body.size();) {
            size_t start = i++;
            if (body[start] == '"' || body[start] == '\'') {
                while (i < body.size()) {
                    char c = body[i++];
                    if (c == '\\' && i < body.size()) ++i;
                    else if (c == body[start]) break;
                }
            } else if (is_ident_char(body[start])) {
                while (i < body.size() && (is_ident_char(body[i]) ||
                       (std::isdigit(static_cast<unsigned char>(body[start])) && body[i] == '.'))) ++i;
            }
            auto token = body.substr(start, i - start);
            auto param = std::find(params.begin(), params.end(), token); // Check if the token matches any macro parameter.
            replacement += param == params.end() ? token : args[param - params.begin()];
        }
        body = std::move(replacement);
    }
    active.emplace_back(id);
    expand_text(ctx, body, out, active); // Recursively expand the macro body with the current active macro list.
    active.pop_back();
}

static void expand_text (LpcCompiler::Context* ctx, std::string_view text,
                         std::vector<char>& out, std::vector<std::string>& active) {
    size_t i = 0;
    ExpansionInput input{
        [&]() -> int { return i < text.size() ? static_cast<unsigned char>(text[i]) : 0; },
        [&]() -> int { return i < text.size() ? static_cast<unsigned char>(text[i++]) : 0; }
    };
    while (i < text.size()) {
        size_t start = i++;
        char c = text[start];
        if (is_ident_start(c)) {
            while (i < text.size() && is_ident_char(text[i])) ++i;
            expand_identifier(ctx, text.substr(start, i - start), input, out, active);
            continue;
        }
        if (c == '"' || c == '\'') {
            while (i < text.size()) {
                char next = text[i++];
                if (next == '\\' && i < text.size()) ++i;
                else if (next == c) break;
            }
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            while (i < text.size() && (is_ident_char(text[i]) || text[i] == '.')) ++i;
        } else if (c == '/' && i < text.size() && (text[i] == '*' || text[i] == '/')) {
            if (text[i] == '/') {
                while (i < text.size() && text[i] != '\n') ++i;
            } else {
                auto end = text.find("*/", i + 1);
                i = end == std::string_view::npos ? text.size() : end + 2;
            }
        }
        out.insert(out.end(), text.begin() + start, text.begin() + i);
    }
}
%}

%option prefix="pp"
%option reentrant
%option noyywrap

%x COMMENT

%%

 /* Directives are consumed; physical newlines are kept for downstream locations. */
^[ \t]*"#"[ \t]*"define"([ \t]+([^\n]|\\\r?\n)*)?   {
    auto operand = directive_operand(yytext, yyleng, "define");
    auto location = yyextra->pp_location;
    location.begin.columns(static_cast<int>(operand.data() - yytext));
    define_macro(yyextra, operand, location);
    yyextra->ppbuffer.insert(yyextra->ppbuffer.end(),
                            std::count(yytext, yytext + yyleng, '\n'), '\n');
}

^[ \t]*"#"[ \t]*"undef"[ \t]+.*    {
    undef_macro(yyextra, directive_operand(yytext, yyleng, "undef"));
}

^[ \t]*"#"[ \t]*"include"[ \t]+.*  { /* TODO: handle include directive */ }

 /* Comments and string literals pass through untouched so '#' inside them is not a directive. */
"//".*                             { EMIT(); return 1; }
"/*"                               { EMIT(); BEGIN(COMMENT); return 1; }
<COMMENT>{
    "*/"                           { EMIT(); BEGIN(INITIAL); return 1; }
    [^*\n]+|"*"|\n                 { EMIT(); return 1; }
    <<EOF>>                        { BEGIN(INITIAL); return 0; }
}
\"([^"\\\n]|\\.)*\"                { EMIT(); return 1; }

[0-9][A-Za-z0-9_.]*                { EMIT(); return 1; }
[A-Za-z_][A-Za-z0-9_]*             {
    // expand the identifier using the preprocessor context (recursively if needed)
    std::vector<std::string> active;
    const std::string id(yytext, yyleng);
    ExpansionInput input{[&] { return scanner_peek(yyscanner); },
                         [&] { return scanner_take(yyscanner); }};
    expand_identifier(yyextra, id, input, yyextra->ppbuffer, active);
    return 1;
}

[^#/"\nA-Za-z_0-9]+|.|\n           { EMIT(); return 1; }

%%

static int scanner_peek (yyscan_t yyscanner) {
    auto* yyg = static_cast<yyguts_t*>(yyscanner);
    auto* ctx = ppget_extra(yyscanner);
    if (ctx->pp_eof) return 0;
    int c = yyinput(yyscanner);
    ctx->pp_eof = c == 0;
    if (c) yyunput(c, yyg->yytext_ptr, yyscanner);
    return c;
}

static int scanner_take (yyscan_t yyscanner) {
    auto* yyg = static_cast<yyguts_t*>(yyscanner);
    auto* ctx = ppget_extra(yyscanner);
    if (ctx->pp_eof) return 0;
    int c = yyinput(yyscanner);
    ctx->pp_eof = c == 0;
    if (c) yy_set_bol(c == '\n');
    if (c == '\n') ctx->pp_location.lines(1);
    else if (c) ctx->pp_location.columns(1);
    return c;
}

/**
 * @brief Preprocessor stage feeding the LPC lexer (called from its YY_INPUT).
 * Fills buf with up to max_len bytes of preprocessed text; returns 0 at end of input.
 */
int preprocessor_feed (char* buf, int max_len, LpcCompiler::Context* ctx) {
    auto scanner = static_cast<yyscan_t>(ctx->ppscanner);
    while (ctx->ppbuffer.empty() && !ctx->pp_eof) {
        if (pplex(scanner) == 0)
            break; // end of input
    }
    int n = static_cast<int>(std::min<size_t>(ctx->ppbuffer.size(), max_len));
    std::copy_n(ctx->ppbuffer.begin(), n, buf);
    ctx->ppbuffer.erase(ctx->ppbuffer.begin(), ctx->ppbuffer.begin() + n);
    SPDLOG_TRACE("preprocessed {} bytes", n);
    return n;
}
