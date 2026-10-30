%{
/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
 */
#include <algorithm>
#include <climits>
#include <string>
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"

#define YY_USER_ACTION \
    yyextra->location.step(); \
    for (int offset = 0; offset < yyleng; ++offset) { \
        if (yytext[offset] == '\n') yyextra->location.lines(1); \
        else yyextra->location.columns(1); \
    } \
    yycolumn = yyextra->location.end.column - 1;

static unsigned int parse_escape (const char* digits, int length,
                                  unsigned int base, unsigned int limit,
                                  const yy::location& location) {
    unsigned int value = 0;
    for (int i = 0; i < length; ++i) {
        const char c = digits[i];
        const unsigned int digit = c <= '9' ? c - '0'
            : c <= 'F' ? c - 'A' + 10 : c - 'a' + 10;
        if (value > (limit - digit) / base) {
            throw yy::LpcParser::syntax_error(location, "Escape sequence out of range");
        }
        value = value * base + digit;
    }
    return value;
}

static void append_utf8 (std::string& out, unsigned int value, const yy::location& location) {
    if (value >= 0xd800 && value <= 0xdfff) {
        throw yy::LpcParser::syntax_error(location, "Invalid Unicode escape sequence");
    }
    if (value <= 0x7f) {
        out += static_cast<char>(value);
    } else if (value <= 0x7ff) {
        out += static_cast<char>(0xc0 | (value >> 6));
        out += static_cast<char>(0x80 | (value & 0x3f));
    } else if (value <= 0xffff) {
        out += static_cast<char>(0xe0 | (value >> 12));
        out += static_cast<char>(0x80 | ((value >> 6) & 0x3f));
        out += static_cast<char>(0x80 | (value & 0x3f));
    } else {
        out += static_cast<char>(0xf0 | (value >> 18));
        out += static_cast<char>(0x80 | ((value >> 12) & 0x3f));
        out += static_cast<char>(0x80 | ((value >> 6) & 0x3f));
        out += static_cast<char>(0x80 | (value & 0x3f));
    }
}
%}

%option reentrant
%option noyywrap
%option yylineno

%x MULTILINE_COMMENT
%x RAW_STRING_LITERAL
%x STRING_LITERAL

/* Printable basic characters except space, parentheses, and backslash. */
RAW_DELIMITER [\x21-\x27\x2a-\x5b\x5d-\x7e]{0,16}

/* Flex declarations section */
%%

"//"([^\n]*)                { /* Match single-line comment, do nothing to strip it */ }
"/*"                        { BEGIN(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"     { BEGIN(INITIAL); }
<MULTILINE_COMMENT>[^*\n]+  { /* Eat comment body in chunks */ }
<MULTILINE_COMMENT>"*"      { /* Eat lone asterisks inside the comment */ }
<MULTILINE_COMMENT>\n       { /* Line tracking is handled by YY_USER_ACTION. */ }
<MULTILINE_COMMENT><<EOF>> {
    BEGIN(INITIAL);
    yyextra->location.step();
    throw yy::LpcParser::syntax_error(yyextra->location, "Unterminated multiline comment");
}

"inherit"       { return yy::LpcParser::make_L_INHERIT(yyextra->location); }
"as"            { return yy::LpcParser::make_LAS(yyextra->location); }
"if"            { return yy::LpcParser::make_L_IF(yyextra->location); }
"else"          { return yy::LpcParser::make_L_ELSE(yyextra->location); }
"while"         { return yy::LpcParser::make_L_WHILE(yyextra->location); }
"do"            { return yy::LpcParser::make_L_DO(yyextra->location); }
"for"           { return yy::LpcParser::make_L_FOR(yyextra->location); }
"foreach"       { return yy::LpcParser::make_L_FOREACH(yyextra->location); }
"in"            { return yy::LpcParser::make_L_IN(yyextra->location); }
"break"         { return yy::LpcParser::make_L_BREAK(yyextra->location); }
"continue"      { return yy::LpcParser::make_L_CONTINUE(yyextra->location); }
"return"        { return yy::LpcParser::make_L_RETURN(yyextra->location); }
"switch"        { return yy::LpcParser::make_L_SWITCH(yyextra->location); }
"case"          { return yy::LpcParser::make_L_CASE(yyextra->location); }
"default"       { return yy::LpcParser::make_L_DEFAULT(yyextra->location); }
"try"           { return yy::LpcParser::make_L_TRY(yyextra->location); }
"catch"         { return yy::LpcParser::make_L_CATCH(yyextra->location); }
"new"           { return yy::LpcParser::make_L_NEW(yyextra->location); }

"void"          { return yy::LpcParser::make_L_TYPE(LpcDataType::T_VOID, yyextra->location); }
"double"        { return yy::LpcParser::make_L_TYPE(LpcDataType::T_DOUBLE, yyextra->location); }
"float"         { return yy::LpcParser::make_L_TYPE(LpcDataType::T_FLOAT, yyextra->location); }
"int"|"bool"    { return yy::LpcParser::make_L_TYPE(LpcDataType::T_INT, yyextra->location); }
"mapping"       { return yy::LpcParser::make_L_TYPE(LpcDataType::T_MAPPING, yyextra->location); }
"object"        { return yy::LpcParser::make_L_TYPE(LpcDataType::T_OBJECT, yyextra->location); }
"string"        { return yy::LpcParser::make_L_TYPE(LpcDataType::T_STRING, yyextra->location); }
"mixed"         { return yy::LpcParser::make_L_TYPE(LpcDataType::T_MIXED, yyextra->location); }

"const"         { return yy::LpcParser::make_L_TYPE_MODIFIER({.b = {.is_const = 1}}, yyextra->location); }
"auto"          { return yy::LpcParser::make_L_TYPE_MODIFIER({.b = {.is_static = 0}}, yyextra->location); }
"static"        { return yy::LpcParser::make_L_TYPE_MODIFIER({.b = {.is_static = 1}}, yyextra->location); }
"private"       { return yy::LpcParser::make_L_TYPE_MODIFIER({.b = {.is_public = 0}}, yyextra->location); }
"public"        { return yy::LpcParser::make_L_TYPE_MODIFIER({.b = {.is_public = 1}}, yyextra->location); }
"virtual"       { return yy::LpcParser::make_L_TYPE_MODIFIER({.b = {.is_nomask = 0}}, yyextra->location); }
"nomask"        { return yy::LpcParser::make_L_TYPE_MODIFIER({.b = {.is_nomask = 1}}, yyextra->location); }

"true"          { return yy::LpcParser::make_L_INTEGER(1, yyextra->location); }
"false"         { return yy::LpcParser::make_L_INTEGER(0, yyextra->location); }

[a-zA-Z_][a-zA-Z0-9_]* {
    /* Identifier */
    return yy::LpcParser::make_L_IDENTIFIER(std::string(yytext), yyextra->location);
}

\" {
    /* Start of string literal */
    yyextra->string_begin = yyextra->location.begin;
    BEGIN(STRING_LITERAL); yyextra->current_string = "";
}
<STRING_LITERAL>\" {
    /* End of string literal */
    BEGIN(INITIAL);
    yyextra->location.begin = yyextra->string_begin;
    return yy::LpcParser::make_L_STRING_LITERAL(yyextra->current_string, yyextra->location);
}
<STRING_LITERAL>\\[0-7]{1,3}|\\x[0-9a-fA-F]+|\\u[0-9a-fA-F]{4}|\\U[0-9a-fA-F]{8} {
    try {
        if (yytext[1] == 'u' || yytext[1] == 'U') {
            append_utf8 (yyextra->current_string, parse_escape(yytext + 2, yyleng - 2, 16, 0x10ffff,
                yyextra->location), yyextra->location);
        } else {
            const bool hex = yytext[1] == 'x';
            const int offset = hex ? 2 : 1;
            append_utf8 (yyextra->current_string, parse_escape(
                yytext + offset, yyleng - offset, hex ? 16 : 8, UCHAR_MAX, yyextra->location), yyextra->location);
        }
    } catch (const yy::LpcParser::syntax_error&) {
        BEGIN(INITIAL);
        throw;
    }
}
<STRING_LITERAL>\\['"?\\abfnrtv] {
    /* Standard C++ simple escape sequences. */
    switch (yytext[1]) {
        case 'a': yyextra->current_string += '\a'; break;
        case 'b': yyextra->current_string += '\b'; break;
        case 'f': yyextra->current_string += '\f'; break;
        case 'n': yyextra->current_string += '\n'; break;
        case 'r': yyextra->current_string += '\r'; break;
        case 't': yyextra->current_string += '\t'; break;
        case 'v': yyextra->current_string += '\v'; break;
        default: yyextra->current_string += yytext[1]; break;
    }
}
<STRING_LITERAL>\\\r?\n {
    /* A backslash-newline continuation contributes no character. */
}
<STRING_LITERAL>\\.|\\ {
    BEGIN(INITIAL);
    throw yy::LpcParser::syntax_error(yyextra->location, "Invalid escape sequence");
}
<STRING_LITERAL>[\r\n] {
    BEGIN(INITIAL);
    throw yy::LpcParser::syntax_error(yyextra->location, "Unescaped newline in string literal");
}
<STRING_LITERAL><<EOF>> {
    BEGIN(INITIAL);
    yyextra->location.step();
    throw yy::LpcParser::syntax_error(yyextra->location, "Unterminated string literal");
}
<STRING_LITERAL>[^\\\r\n\"]+ {
    /* Append the matched text to the current string */
    yyextra->current_string.append(yytext, yyleng);
}

(u8|u|U|L)?R\"{RAW_DELIMITER}\( {
    /* Start of raw string literal */
    yyextra->string_begin = yyextra->location.begin;
    const std::string opening(yytext, yyleng);
    const auto delimiter_start = opening.find('"') + 1;
    yyextra->raw_string_delimiter = opening.substr(delimiter_start, opening.size() - delimiter_start - 1);
    BEGIN(RAW_STRING_LITERAL); yyextra->current_string = "";
}
<RAW_STRING_LITERAL>\){RAW_DELIMITER}\" {
    /* Only the opening delimiter can terminate this raw string. */
    const std::string terminator = ")" + yyextra->raw_string_delimiter + '"';
    if (std::string(yytext, yyleng).compare(0, terminator.size(), terminator) == 0) {
        yyless(terminator.size());
        yyextra->location.end = yyextra->location.begin;
        yyextra->location.columns(yyleng);
        yycolumn = yyextra->location.end.column - 1;
        BEGIN(INITIAL);
        yyextra->location.begin = yyextra->string_begin;
        return yy::LpcParser::make_L_STRING_LITERAL(yyextra->current_string, yyextra->location);
    }
    /* Rescan after ')' so an overlapping terminator is not missed. */
    yyless(1);
    yyextra->location.end = yyextra->location.begin;
    yyextra->location.columns(1);
    yycolumn = yyextra->location.end.column - 1;
    yyextra->current_string += ')';
}
<RAW_STRING_LITERAL>[^)]+ {
    /* Append the matched text (no escape sequences, newlines are preserved) to the current raw string */
    yyextra->current_string.append(yytext, yyleng);
}
<RAW_STRING_LITERAL>\) {
    yyextra->current_string += ')';
}
<RAW_STRING_LITERAL><<EOF>> {
    BEGIN(INITIAL);
    yyextra->location.step();
    throw yy::LpcParser::syntax_error(yyextra->location, "Unterminated raw string literal");
}

[0-9]+\\.[0-9]+([eE][+-]?[0-9]+)? { return yy::LpcParser::make_L_REAL_NUMBER(std::stod(yytext), yyextra->location); }
[0-9]+[eE][+-]?[0-9]+ { return yy::LpcParser::make_L_REAL_NUMBER(std::stod(yytext), yyextra->location); }

[0-9]+ { return yy::LpcParser::make_L_INTEGER(std::stol(yytext), yyextra->location); }

"+"|"-"|"*"|"/"|"%"|"&"|"|"|"^" {
    /* binary operators */
    return yy::LpcParser::symbol_type(yytext[0], yyextra->location);
}

"=="    { return yy::LpcParser::make_L_ORDER(static_cast<int>(yy::LpcOrder::EQUAL), yyextra->location); }
"!="    { return yy::LpcParser::make_L_ORDER(static_cast<int>(yy::LpcOrder::NOT_EQUAL), yyextra->location); }
"++"    { return yy::LpcParser::make_L_INC(yyextra->location); }
"--"    { return yy::LpcParser::make_L_DEC(yyextra->location); }
"&&"    { return yy::LpcParser::make_L_LAND(yyextra->location); }
"||"    { return yy::LpcParser::make_L_LOR(yyextra->location); }
"!"     { return yy::LpcParser::make_L_NOT(yyextra->location); }
"<<"    { return yy::LpcParser::make_L_LSH(yyextra->location); }
">>"    { return yy::LpcParser::make_L_RSH(yyextra->location); }
">="    { return yy::LpcParser::make_L_ORDER(static_cast<int>(yy::LpcOrder::GREATER_EQUAL), yyextra->location); }
">"     { return yy::LpcParser::make_L_ORDER(static_cast<int>(yy::LpcOrder::GREATER), yyextra->location); }
"<="    { return yy::LpcParser::make_L_ORDER(static_cast<int>(yy::LpcOrder::LESS_EQUAL), yyextra->location); }
"<"     { return yy::LpcParser::make_L_ORDER(static_cast<int>(yy::LpcOrder::LESS), yyextra->location); }
"="     { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::ASSIGN), yyextra->location); }
"+="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::ADD_ASSIGN), yyextra->location); }
"-="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::SUB_ASSIGN), yyextra->location); }
"*="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::MUL_ASSIGN), yyextra->location); }
" /="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::DIV_ASSIGN), yyextra->location); }
"%="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::MOD_ASSIGN), yyextra->location); }
"&="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::AND_ASSIGN), yyextra->location); }
"|="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::OR_ASSIGN), yyextra->location); }
"^="    { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(yy::LpcAssign::XOR_ASSIGN), yyextra->location); }

"..."   { return yy::LpcParser::make_L_ELLIPSIS(yyextra->location); }

","|";"|"."|"{"|"}"|"("|")" {
    /* punctuation symbols */
    return yy::LpcParser::symbol_type(yytext[0], yyextra->location);
}

[ \t\n] { /* ignore whitespace */ }

. {
    /* Catch-all for any other character */
    throw yy::LpcParser::syntax_error (yyextra->location, "Unexpected character");
}

<<EOF>> {
    yyextra->location.step();
    return yy::LpcParser::make_YYEOF(yyextra->location);
}

%%

// User code section (optional)
