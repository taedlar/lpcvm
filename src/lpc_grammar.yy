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

%code requires {
    #include <string>
    #include <cstdint>

    enum class LpcType: uint32_t {
        T_DOUBLE,
        T_FLOAT,
        T_INT,
        T_MAPPING,
        T_OBJECT,
        T_STRING,
        T_NOMASK = 1 << 8,
        T_PRIVATE = 1 << 9
    };
}

%token <int> L_INTEGER
%token <std::string> L_STRING_LITERAL
%token <LpcType> L_TYPE L_TYPE_MODIFIER

%token L_PLUS L_MINUS L_TIMES L_DIVIDE
%left L_PLUS L_MINUS L_TIMES L_DIVIDE

%token L_IF L_ELSE
%token L_WHILE L_DO L_FOR L_FOREACH L_IN
%token L_BREAK L_CONTINUE
%token L_RETURN
%token L_SWITCH L_CASE L_DEFAULT
%token L_TRY L_CATCH
%token L_NEW

%token <std::string> L_IDENTIFIER

%code {
    #include <iostream>
}

%%

// Grammar rules section
expr:
      L_INTEGER
    | expr L_PLUS expr
    ;

%%

// User subroutines section
void yy::LpcParser::error(const std::string& message) {
    std::cerr << message << '\n';
}
