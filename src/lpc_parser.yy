%require "3.2"
%language "C++"
%output "lpc_parser.cpp"
%define api.token.constructor
%define api.value.type variant
%define api.parser.class {LpcParser}

// Bison Declarations Section
%code provides {
    yy::LpcParser::symbol_type yylex (yyscan_t yyscanner);
}

%code requires {
    /*
    * SPDX-License-Identifier: MIT
    * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
    */
    #include <string>
    #include <cstdint>

    #ifndef YY_TYPEDEF_YY_SCANNER_T
    #define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
    #endif
    enum class LpcType: uint32_t {
        T_VOID = 0,
        T_DOUBLE,
        T_FLOAT,
        T_INT,
        T_MAPPING,
        T_OBJECT,
        T_STRING,
        T_MIXED = 127,
        T_ARRAY = 1 << 7,
        T_STATIC = 1 << 8,
        T_PRIVATE = 1 << 9,
        T_PUBLIC = 1 << 10,
        T_NOMASK = 1 << 11,
    };

    enum class LpcAssign: int {
        ASSIGN = 0,
        ADD_ASSIGN,
        SUB_ASSIGN,
        MUL_ASSIGN,
        DIV_ASSIGN,
        MOD_ASSIGN,
        LSH_ASSIGN,
        RSH_ASSIGN,
        AND_ASSIGN,
        XOR_ASSIGN,
        OR_ASSIGN,
    };

    enum class LpcOrder: int {
        LESS = '<',
        LESS_EQUAL,
        GREATER,
        GREATER_EQUAL,
    };
}

// expected parameter for yylex()
%param { yyscan_t yyscanner }

%token L_INHERIT
%token L_IF L_ELSE
%token L_WHILE L_DO L_FOR L_FOREACH L_IN
%token L_BREAK L_CONTINUE
%token L_RETURN
%token L_SWITCH L_CASE L_DEFAULT
%token L_TRY L_CATCH
%token L_NEW

%nonassoc LOWER_THAN_ELSE
%nonassoc L_ELSE

%token <std::string> L_IDENTIFIER

%token <int> L_INTEGER
%token <double> L_REAL_NUMBER
%token <std::string> L_STRING_LITERAL
%token <LpcType> L_TYPE L_TYPE_MODIFIER
%token <int> L_ASSIGN L_ORDER

// Operator precedence and associativity
%right L_ASSIGN
%right '?'
%left L_LOR
%left L_LAND
%left '|'
%left '^'
%left '&'
%left L_EQ L_NE
%left L_ORDER '<'
%left L_LSH L_RSH
%left '+' '-'
%left '*' '%' '/'
%right L_NOT '~'
%nonassoc L_INC L_DEC

%token L_ELLIPSIS

%code {
    #include <iostream>
}

%type <uint32_t> type_modifier_list optional_type type optional_star
%type <std::string> str_literal str_const

%%

// Grammar rules section
program
    : program def extra_semicolon
    | /* empty */
    ;

extra_semicolon
    : /*empty*/
    | ';' { /* ignore extra semicolon(s) between definitions */ }
    ;

def
    : type_modifier_list optional_type optional_star L_IDENTIFIER
        {
            /* combine the type modifiers, optional type, and optional star into a single type representation */
            uint32_t combined_type = static_cast<uint32_t>($1) | static_cast<uint32_t>($2) | static_cast<uint32_t>($3);
        }
      '(' optional_argument_list ')'
        {
            /* handle function arguments */
        }
      block_or_semicolon
        {
            /* handle forward declaration or definition for the function */
        }
    | inheritance
    ;

inheritance
    : type_modifier_list L_INHERIT str_const ';'
    ;

type_modifier_list
    : /* empty */
        {
            $$ = 0;
        }
    | type_modifier_list L_TYPE_MODIFIER
        {
            /* combine the type modifiers using bitwise OR */
            $$ = $1 | static_cast<uint32_t>($2);
        }
    ;

optional_type
    : /* empty */
        {
            /* implicit rule: if no type is specified, default to int */
            $$ = static_cast<uint32_t>(LpcType::T_INT);
        }
    | type
        {
            $$ = static_cast<uint32_t>($1);
        }
    ;

type
    : L_TYPE
        { $$ = static_cast<uint32_t>($1); }
    ;

optional_star
    : /* empty */
        { $$ = 0; }
    | '*'
        { $$ = static_cast<uint32_t>(LpcType::T_ARRAY); }
    ;

str_const
    : L_STRING_LITERAL
        {
            $$ = $1;
        }
    | '(' str_const ')'
        {
            /* handle parentheses around string constants */
            $$ = $2;
        }
    | str_const L_STRING_LITERAL
        {
            /* append the string literal to the existing string */
            $$ += $2;
        }
    ;

integer
    : L_INTEGER
    ;

real_number
    : L_REAL_NUMBER
    ;

str_literal
    : L_STRING_LITERAL
    | str_literal L_STRING_LITERAL
    ;

optional_argument_list
    : /* empty */
    | argument_list
    ;

argument_list
    : one_argument
    | argument_list ',' one_argument
    | argument_list L_ELLIPSIS
    ;

one_argument
    : type optional_star
    | type optional_star L_IDENTIFIER
    ;

block_or_semicolon
    : block
    | ';'
    ;

block
    : '{' stmt_list '}'
    ;

stmt_list
    : /* empty */
    | stmt_list stmt
    ;

stmt
    : comma_expr ';'
    | if_stmt
    | while_stmt
    | do_stmt
    | return_stmt
    | block
    | /* empty */ ';'
    | L_BREAK ';'
    | L_CONTINUE ';'
    ;

comma_expr
    : expr0
    | comma_expr ',' expr0
    ;

expr0
    : lvalue L_ASSIGN expr0
    | expr0 '?' expr0 ':' expr0 %prec '?'
    | expr0 L_LOR expr0
    | expr0 L_LAND expr0
    | expr0 '|' expr0
    | expr0 '^' expr0
    | expr0 '&' expr0
    | expr0 L_EQ expr0
    | expr0 L_NE expr0
    | expr0 L_ORDER expr0
    | expr0 '<' expr0
    | expr0 L_LSH expr0
    | expr0 L_RSH expr0
    | expr0 '+' expr0
    | expr0 '-' expr0
    | expr0 '*' expr0
    | expr0 '/' expr0
    | expr0 '%' expr0
    | L_INC lvalue %prec L_NOT
    | L_DEC lvalue %prec L_NOT
    | lvalue L_INC
    | lvalue L_DEC
    | L_NOT expr0
    | '~' expr0
    | '-' expr0 %prec L_NOT
    | expr4
    | integer
    | real_number
    | str_literal
    ;

lvalue
    : expr4
    ;

expr4
    : '(' comma_expr ')'
    | L_IDENTIFIER
    | function_call
    | expr4 '[' comma_expr ']'
    ;

if_stmt
    : L_IF '(' comma_expr ')' stmt optional_else_stmt
    ;

optional_else_stmt
    : /* empty */ %prec LOWER_THAN_ELSE
    | L_ELSE stmt
    ;

while_stmt
    : L_WHILE '(' comma_expr ')' stmt
    ;

do_stmt
    : L_DO stmt L_WHILE '(' comma_expr ')' ';'
    ;

return_stmt
    : L_RETURN ';'
    | L_RETURN comma_expr ';'
    ;

function_call
    : L_IDENTIFIER '(' expr_list ')'
    ;

expr_list
    : /* empty */
    | expr_list ',' expr0
    ;

%%

// User subroutines section
void yy::LpcParser::error (const std::string& message) {
    std::cerr << message << '\n';
}
