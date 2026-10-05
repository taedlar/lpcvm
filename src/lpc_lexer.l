%option noyywrap

%{
#include "lpc_grammar.hpp"
#include <string>
#define YY_DECL yy::LpcParser::symbol_type yylex()
%}

/* Flex declarations section */
%%

"[0-9]+"        { return yy::LpcParser::make_NUMBER(std::stoi(yytext)); }
"+"             { return yy::LpcParser::make_PLUS(); }
[ \t\n]         {/* ignore whitespace */}
.               { throw yy::LpcParser::syntax_error("Invalid character"); }

%%

// User code section (optional)
