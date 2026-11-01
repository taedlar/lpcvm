%{
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "lpc_compiler.hpp"
#include "symbol_table.hpp"
#include <algorithm>
#include <cctype>
#include <string_view>

// append the matched text to the preprocessed output
#define EMIT() yyextra->ppbuffer.insert(yyextra->ppbuffer.end(), yytext, yytext + yyleng)

int preprocessor_feed (char* buf, int max_len, LpcCompiler::Context* ctx);

static bool is_ident_start (char c) { return std::isalpha(static_cast<unsigned char>(c)) || c == '_'; }
static bool is_ident_char (char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

// Parse "#define NAME body" / "#undef NAME" text: returns the directive operand (after the keyword).
static std::string_view directive_operand (const char* text, int len, std::string_view keyword) {
    std::string_view line(text, len);
    auto pos = line.find(keyword);
    line.remove_prefix(pos + keyword.size());
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.remove_prefix(1);
    return line;
}

static void define_macro (LpcCompiler::Context* ctx, std::string_view operand) {
    size_t n = 0;
    while (n < operand.size() && is_ident_char(operand[n])) n++;
    if (n == 0 || !is_ident_start(operand[0])) {
        SPDLOG_WARN("invalid macro name in #define");
        return;
    }
    std::string name(operand.substr(0, n));
    std::string_view body = operand.substr(n);
    if (!body.empty() && (body.front() == '(')) {
        SPDLOG_WARN("function-like macro '{}' is not supported", name);
        return;
    }
    if (auto c = body.find("//"); c != std::string_view::npos) body = body.substr(0, c);
    while (!body.empty() && std::isspace(static_cast<unsigned char>(body.front()))) body.remove_prefix(1);
    while (!body.empty() && std::isspace(static_cast<unsigned char>(body.back()))) body.remove_suffix(1);

    SymbolSignature sig{};
    sig.b.is_const = 1;
    auto entry = std::make_shared<SymbolTable::Entry>(name, sig);
    entry->value = std::string(body);
    ctx->defines->emplace(name, entry);
}

static void undef_macro (LpcCompiler::Context* ctx, std::string_view operand) {
    size_t n = 0;
    while (n < operand.size() && is_ident_char(operand[n])) n++;
    if (n > 0) ctx->defines->erase(operand.substr(0, n));
}

// Append the expansion of identifier 'id' to out; macros are rescanned, but a macro is never re-expanded inside itself.
static void expand_identifier (LpcCompiler::Context* ctx, const std::string& id, std::vector<char>& out, std::vector<std::string>& active) {
    std::shared_ptr<SymbolTable::Entry> entry = ctx->defines ? ctx->defines->lookup(id) : nullptr;
    auto* body = entry && entry->value ? std::get_if<std::string>(&*entry->value) : nullptr;
    if (!body || std::find(active.begin(), active.end(), id) != active.end()) {
        out.insert(out.end(), id.begin(), id.end());
        return;
    }
    active.push_back(id);
    for (size_t i = 0; i < body->size();) {
        char c = (*body)[i];
        if (is_ident_start(c)) {
            size_t j = i;
            while (j < body->size() && is_ident_char((*body)[j])) j++;
            expand_identifier(ctx, body->substr(i, j - i), out, active);
            i = j;
        } else if (c == '"') { // copy string literals verbatim
            size_t j = i + 1;
            while (j < body->size() && (*body)[j] != '"') j += ((*body)[j] == '\\') ? 2 : 1;
            j = std::min(j + 1, body->size());
            out.insert(out.end(), body->begin() + i, body->begin() + j);
            i = j;
        } else if (std::isdigit(static_cast<unsigned char>(c))) { // numbers, so suffixes are not taken as identifiers
            size_t j = i;
            while (j < body->size() && (is_ident_char((*body)[j]) || (*body)[j] == '.')) j++;
            out.insert(out.end(), body->begin() + i, body->begin() + j);
            i = j;
        } else {
            out.push_back(c);
            i++;
        }
    }
    active.pop_back();
}
%}

%option prefix="pp"
%option reentrant
%option noyywrap
%option nounput
%option noinput

%x COMMENT

%%

 /* Directives are consumed; the newline is kept so line numbers stay in sync. */
^[ \t]*"#"[ \t]*"define"[ \t]+.*   {
    define_macro(yyextra, directive_operand(yytext, yyleng, "define"));
}

^[ \t]*"#"[ \t]*"undef"[ \t]+.*    { undef_macro(yyextra, directive_operand(yytext, yyleng, "undef")); }
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
    std::vector<std::string> active;
    expand_identifier(yyextra, std::string(yytext, yyleng), yyextra->ppbuffer, active);
    return 1;
}

[^#/"\nA-Za-z_0-9]+|.|\n           { EMIT(); return 1; }

%%

/**
 * Preprocessor stage feeding the LPC lexer (called from its YY_INPUT).
 * Fills buf with up to max_len bytes of preprocessed text; returns 0 at end of input.
 */
int preprocessor_feed (char* buf, int max_len, LpcCompiler::Context* ctx) {
    auto scanner = static_cast<yyscan_t>(ctx->ppscanner);
    while (ctx->ppbuffer.empty()) {
        if (pplex(scanner) == 0)
            break; // end of input
    }
    int n = static_cast<int>(std::min<size_t>(ctx->ppbuffer.size(), max_len));
    std::copy_n(ctx->ppbuffer.begin(), n, buf);
    ctx->ppbuffer.erase(ctx->ppbuffer.begin(), ctx->ppbuffer.begin() + n);
    SPDLOG_TRACE("preprocessed {} bytes", n);
    return n;
}
