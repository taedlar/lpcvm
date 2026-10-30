%{
// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "lpc_compiler.hpp"

// append the matched text to the preprocessed output
#define EMIT() yyextra->ppbuffer.insert(yyextra->ppbuffer.end(), yytext, yytext + yyleng)

int preprocess (char* buf, int max_len, LpcCompiler::Context* ctx);
%}

%option prefix="pp"
%option reentrant
%option noyywrap
%option nounput
%option noinput

%x COMMENT

%%

 /* Directives are consumed; the newline is kept so line numbers stay in sync. */
^[ \t]*"#"[ \t]*"define"[ \t]+.*   { /* TODO: handle define directive */ }
^[ \t]*"#"[ \t]*"undef"[ \t]+.*    { /* TODO: handle undef directive */ }
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

[^#/"\n]+|.|\n                     { EMIT(); return 1; }

%%

/**
 * Preprocessor stage feeding the LPC lexer (called from its YY_INPUT).
 * Fills buf with up to max_len bytes of preprocessed text; returns 0 at end of input.
 */
int preprocess (char* buf, int max_len, LpcCompiler::Context* ctx) {
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
