%option noyywrap

%{
/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
 */
#include "lpc_parser.hpp"
#include <climits>
#include <string>
#define YY_DECL yy::LpcParser::symbol_type yylex()

std::string current_string;
std::string raw_string_delimiter;

static unsigned int parse_escape(const char* digits, int length,
                                 unsigned int base, unsigned int limit) {
    unsigned int value = 0;
    for (int i = 0; i < length; ++i) {
        const char c = digits[i];
        const unsigned int digit = c <= '9' ? c - '0'
            : c <= 'F' ? c - 'A' + 10 : c - 'a' + 10;
        if (value > (limit - digit) / base) {
            throw yy::LpcParser::syntax_error("Escape sequence out of range");
        }
        value = value * base + digit;
    }
    return value;
}

static void append_utf8(unsigned int value) {
    if (value >= 0xd800 && value <= 0xdfff) {
        throw yy::LpcParser::syntax_error("Invalid Unicode escape sequence");
    }
    if (value <= 0x7f) {
        current_string += static_cast<char>(value);
    } else if (value <= 0x7ff) {
        current_string += static_cast<char>(0xc0 | (value >> 6));
        current_string += static_cast<char>(0x80 | (value & 0x3f));
    } else if (value <= 0xffff) {
        current_string += static_cast<char>(0xe0 | (value >> 12));
        current_string += static_cast<char>(0x80 | ((value >> 6) & 0x3f));
        current_string += static_cast<char>(0x80 | (value & 0x3f));
    } else {
        current_string += static_cast<char>(0xf0 | (value >> 18));
        current_string += static_cast<char>(0x80 | ((value >> 12) & 0x3f));
        current_string += static_cast<char>(0x80 | ((value >> 6) & 0x3f));
        current_string += static_cast<char>(0x80 | (value & 0x3f));
    }
}
%}

%x MULTILINE_COMMENT
%x RAW_STRING_LITERAL
%x L_STRING_LITERAL

/* Printable basic characters except space, parentheses, and backslash. */
RAW_DELIMITER [\x21-\x27\x2a-\x5b\x5d-\x7e]{0,16}

/* Flex declarations section */
%%

"//"([^\n]*)                { /* Match single-line comment, do nothing to strip it */ }
"/*"                        { BEGIN(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>"*/"     { BEGIN(INITIAL); }
<MULTILINE_COMMENT>[^*\n]+  { /* Eat comment body in chunks */ }
<MULTILINE_COMMENT>"*"      { /* Eat lone asterisks inside the comment */ }
<MULTILINE_COMMENT>\n       { /* Preserve newlines so compiler error lines stay accurate */ }
<MULTILINE_COMMENT><<EOF>> {
    BEGIN(INITIAL);
    throw yy::LpcParser::syntax_error("Unterminated multiline comment");
}

"inherit"       { return yy::LpcParser::make_L_INHERIT(); }
"if"            { return yy::LpcParser::make_L_IF(); }
"else"          { return yy::LpcParser::make_L_ELSE(); }
"while"         { return yy::LpcParser::make_L_WHILE(); }
"do"            { return yy::LpcParser::make_L_DO(); }
"for"           { return yy::LpcParser::make_L_FOR(); }
"foreach"       { return yy::LpcParser::make_L_FOREACH(); }
"in"            { return yy::LpcParser::make_L_IN(); }
"break"         { return yy::LpcParser::make_L_BREAK(); }
"continue"      { return yy::LpcParser::make_L_CONTINUE(); }
"return"        { return yy::LpcParser::make_L_RETURN(); }
"switch"        { return yy::LpcParser::make_L_SWITCH(); }
"case"          { return yy::LpcParser::make_L_CASE(); }
"default"       { return yy::LpcParser::make_L_DEFAULT(); }
"try"           { return yy::LpcParser::make_L_TRY(); }
"catch"         { return yy::LpcParser::make_L_CATCH(); }
"new"           { return yy::LpcParser::make_L_NEW(); }

"void"          { return yy::LpcParser::make_L_TYPE(LpcType::T_VOID); }
"double"        { return yy::LpcParser::make_L_TYPE(LpcType::T_DOUBLE); }
"float"         { return yy::LpcParser::make_L_TYPE(LpcType::T_FLOAT); }
"int"|"bool"    { return yy::LpcParser::make_L_TYPE(LpcType::T_INT); }
"mapping"       { return yy::LpcParser::make_L_TYPE(LpcType::T_MAPPING); }
"object"        { return yy::LpcParser::make_L_TYPE(LpcType::T_OBJECT); }
"string"        { return yy::LpcParser::make_L_TYPE(LpcType::T_STRING); }
"mixed"         { return yy::LpcParser::make_L_TYPE(LpcType::T_MIXED); }
"static"        { return yy::LpcParser::make_L_TYPE_MODIFIER(LpcType::T_STATIC); }
"private"       { return yy::LpcParser::make_L_TYPE_MODIFIER(LpcType::T_PRIVATE); }
"public"        { return yy::LpcParser::make_L_TYPE_MODIFIER(LpcType::T_PUBLIC); }
"nomask"        { return yy::LpcParser::make_L_TYPE_MODIFIER(LpcType::T_NOMASK); }

"true"          { return yy::LpcParser::make_L_INTEGER(1); }
"false"         { return yy::LpcParser::make_L_INTEGER(0); }

[a-zA-Z_][a-zA-Z0-9_]* {
    /* Identifier */
    return yy::LpcParser::make_L_IDENTIFIER(std::string(yytext));
}

\" {
    /* Start of string literal */
    BEGIN(L_STRING_LITERAL); current_string = "";
}
<L_STRING_LITERAL>\" {
    /* End of string literal */
    BEGIN(INITIAL); return yy::LpcParser::make_L_STRING_LITERAL(current_string);
}
<L_STRING_LITERAL>\\[0-7]{1,3}|\\x[0-9a-fA-F]+|\\u[0-9a-fA-F]{4}|\\U[0-9a-fA-F]{8} {
    try {
        if (yytext[1] == 'u' || yytext[1] == 'U') {
            append_utf8(parse_escape(yytext + 2, yyleng - 2, 16, 0x10ffff));
        } else {
            const bool hex = yytext[1] == 'x';
            const int offset = hex ? 2 : 1;
            current_string += static_cast<char>(parse_escape(
                yytext + offset, yyleng - offset, hex ? 16 : 8, UCHAR_MAX));
        }
    } catch (const yy::LpcParser::syntax_error&) {
        BEGIN(INITIAL);
        throw;
    }
}
<L_STRING_LITERAL>\\['"?\\abfnrtv] {
    /* Standard C++ simple escape sequences. */
    switch (yytext[1]) {
        case 'a': current_string += '\a'; break;
        case 'b': current_string += '\b'; break;
        case 'f': current_string += '\f'; break;
        case 'n': current_string += '\n'; break;
        case 'r': current_string += '\r'; break;
        case 't': current_string += '\t'; break;
        case 'v': current_string += '\v'; break;
        default: current_string += yytext[1]; break;
    }
}
<L_STRING_LITERAL>\\\r?\n {
    /* A backslash-newline continuation contributes no character. */
}
<L_STRING_LITERAL>\\.|\\ {
    BEGIN(INITIAL);
    throw yy::LpcParser::syntax_error("Invalid escape sequence");
}
<L_STRING_LITERAL>[\r\n] {
    BEGIN(INITIAL);
    throw yy::LpcParser::syntax_error("Unescaped newline in string literal");
}
<L_STRING_LITERAL><<EOF>> {
    BEGIN(INITIAL);
    throw yy::LpcParser::syntax_error("Unterminated string literal");
}
<L_STRING_LITERAL>[^\\\r\n\"]+ {
    /* Append the matched text to the current string */
    current_string.append(yytext, yyleng);
}

(u8|u|U|L)?R\"{RAW_DELIMITER}\( {
    /* Start of raw string literal */
    const std::string opening(yytext, yyleng);
    const auto delimiter_start = opening.find('"') + 1;
    raw_string_delimiter = opening.substr(delimiter_start, opening.size() - delimiter_start - 1);
    BEGIN(RAW_STRING_LITERAL); current_string = "";
}
<RAW_STRING_LITERAL>\){RAW_DELIMITER}\" {
    /* Only the opening delimiter can terminate this raw string. */
    const std::string terminator = ")" + raw_string_delimiter + '"';
    if (std::string(yytext, yyleng).compare(0, terminator.size(), terminator) == 0) {
        yyless(terminator.size());
        BEGIN(INITIAL); return yy::LpcParser::make_L_STRING_LITERAL(current_string);
    }
    /* Rescan after ')' so an overlapping terminator is not missed. */
    yyless(1);
    current_string += ')';
}
<RAW_STRING_LITERAL>[^)]+ {
    /* Append the matched text (no escape sequences, newlines are preserved) to the current raw string */
    current_string.append(yytext, yyleng);
}
<RAW_STRING_LITERAL>\) {
    current_string += ')';
}
<RAW_STRING_LITERAL><<EOF>> {
    BEGIN(INITIAL);
    throw yy::LpcParser::syntax_error("Unterminated raw string literal");
}

"[0-9]+\\.[0-9]+([eE][+-]?[0-9]+)?" { return yy::LpcParser::make_L_REAL_NUMBER(std::stod(yytext)); }
"[0-9]+[eE][+-]?[0-9]+" { return yy::LpcParser::make_L_REAL_NUMBER(std::stod(yytext)); }

"[0-9]+"        { return yy::LpcParser::make_L_INTEGER(std::stoi(yytext)); }

"+"|"-"|"*"|"/"|"%"|"&"|"|"|"^" {
    /* binary operators */
    return yytext[0];
}

"=="            { return yy::LpcParser::make_L_EQ(); }
"!="            { return yy::LpcParser::make_L_NE(); }
"="             { return yy::LpcParser::make_L_ASSIGN(static_cast<int>(LpcAssign::ASSIGN)); }
"++"            { return yy::LpcParser::make_L_INC(); }
"--"            { return yy::LpcParser::make_L_DEC(); }
"&&"            { return yy::LpcParser::make_L_LAND(); }
"||"            { return yy::LpcParser::make_L_LOR(); }
"!"             { return yy::LpcParser::make_L_NOT(); }
"<<"            { return yy::LpcParser::make_L_LSH(); }
">>"            { return yy::LpcParser::make_L_RSH(); }
">="            { return yy::LpcParser::make_L_ORDER(static_cast<int>(LpcOrder::GREATER_EQUAL)); }
">"             { return yy::LpcParser::make_L_ORDER(static_cast<int>(LpcOrder::GREATER)); }
"<="            { return yy::LpcParser::make_L_ORDER(static_cast<int>(LpcOrder::LESS_EQUAL)); }
"<"             { return yy::LpcParser::make_L_ORDER(static_cast<int>(LpcOrder::LESS)); }

"..."           { return yy::LpcParser::make_L_ELLIPSIS(); }

","|";"|"."|"{"|"}"|"("|")" {
    /* punctuation symbols */
    return yytext[0];
}

[ \t\n]         {/* ignore whitespace */}

. {
    /* Catch-all for any other character, can be used for error handling */
    throw yy::LpcParser::syntax_error("Unexpected character");
}

%%

// User code section (optional)
