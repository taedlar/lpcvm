%require "3.2"
%language "C++"
%defines "lpc_grammar.hpp"
%output "lpc_grammar.cpp"
%define api.token.constructor
%define api.value.type variant
%define api.parser.class {LpcParser}

// Bison Declarations Section
%code provides {
    yy::LpcParser::symbol_type yylex();
}

%token <int> NUMBER
%token PLUS
%left PLUS

%code {
  #include <iostream>
}

%%

// Grammar rules section
expr:
      NUMBER
    | expr PLUS expr
    ;

%%

// User subroutines section
void yy::LpcParser::error(const std::string& message) {
  std::cerr << message << '\n';
}
