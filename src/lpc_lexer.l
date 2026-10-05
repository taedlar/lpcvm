%option noyywrap

%{
#include "lpc_grammar.hpp"
#include <string>
#define YY_DECL yy::LpcParser::symbol_type yylex()

std::string current_string;
%}

%x L_STRING_LITERAL

/* Flex declarations section */
%%

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

"float"         { return yy::LpcParser::make_L_TYPE(LpcType::T_FLOAT); }
"int"           { return yy::LpcParser::make_L_TYPE(LpcType::T_INT); }
"mapping"       { return yy::LpcParser::make_L_TYPE(LpcType::T_MAPPING); }
"object"        { return yy::LpcParser::make_L_TYPE(LpcType::T_OBJECT); }
"string"        { return yy::LpcParser::make_L_TYPE(LpcType::T_STRING); }
"nomask"        { return yy::LpcParser::make_L_TYPE_MODIFIER(LpcType::T_NOMASK); }
"private"|"static"       { return yy::LpcParser::make_L_TYPE_MODIFIER(LpcType::T_PRIVATE); }

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
<L_STRING_LITERAL>\\. {
    /* Handle escape sequences in string literal */
    switch (yytext[1]) {
        case 'n': current_string += '\n'; break;
        case 'r': current_string += '\r'; break;
        case 't': current_string += '\t'; break;
        case '\"': current_string += '\"'; break;
        case '\\': current_string += '\\'; break;
        default: current_string += yytext[1]; break;
    }
}
<L_STRING_LITERAL>[^\\\n\"]+ {
    /* Append the matched text to the current string */
    current_string += yytext;
}

"[0-9]+"        { return yy::LpcParser::make_L_INTEGER(std::stoi(yytext)); }

"+"             { return yy::LpcParser::make_L_PLUS(); }
"-"             { return yy::LpcParser::make_L_MINUS(); }
"*"             { return yy::LpcParser::make_L_TIMES(); }
"/"             { return yy::LpcParser::make_L_DIVIDE(); }

";"|"{"|"}"|"("|")"     { return yytext[0]; }

[ \t\n]         {/* ignore whitespace */}

%%

// User code section (optional)
