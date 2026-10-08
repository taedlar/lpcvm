%require "3.2"
%language "C++"
%output "lpc_parser.cpp"
%define api.token.constructor
%define api.value.type variant
%define api.parser.class {LpcParser}
%lex-param { yyscan_t yyscanner }
%parse-param { yyscan_t yyscanner }

%code requires {
    /*
    * SPDX-License-Identifier: MIT
    * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
    */
    #include <cstdint>

    namespace yy {
        // bit-OR flags for variable or function return types and modifiers
        enum class LpcType: uint32_t {
            T_VOID = 0, // function only
            T_DOUBLE,
            T_FLOAT,
            T_INT,
            T_MAPPING,
            T_OBJECT,
            T_STRING,
            T_MIXED = 127,
            T_ARRAY = 1 << 7,
            T_AUTO = 0,
            T_STATIC = 1 << 8, // on: blueprint-owned, shared by all clones; off: clone-specific or local variables
            T_PRIVATE = 0,
            T_PUBLIC = 1 << 9, // on: visible to `->` operator; off: not visible to `->` operator
            T_VIRTUAL = 0,
            T_NOMASK = 1 << 10, // on: cannot be overridden by inheritance; off: can be overridden by inheritance
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
            LESS = 0,
            LESS_EQUAL,
            GREATER,
            GREATER_EQUAL,
        };
    } // namespace yy

    // typedef yyscan_t for the reentrant lexer handle
    #ifndef YY_TYPEDEF_YY_SCANNER_T
    #define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
    #endif
}

%code provides {
    // forward declarations for lexer functions (reentrant)
    #undef YY_DECL
    #define YY_DECL yy::LpcParser::symbol_type yylex (yyscan_t yyscanner)
    extern yy::LpcParser::symbol_type yylex (yyscan_t yyscanner);

    // forward declarations for lexer helper functions
    #undef YY_EXTRA_TYPE
    #define YY_EXTRA_TYPE LpcCompiler::Context*
    extern YY_EXTRA_TYPE yyget_extra(yyscan_t yyscanner);

    extern int yyget_lineno (yyscan_t yyscanner);
    extern int yyget_column (yyscan_t yyscanner);
}

%code top {
    /*
    * SPDX-License-Identifier: MIT
    * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
    */
    #include "ast_builder.hpp"
    #include "lpc_compiler.hpp"
    #include <stdexcept>
    #include <string>
    #include <memory>

    class AstProgramNode: public AstNode {
    public:
        AstProgramNode();
        void generate_code(std::vector<uint8_t>& code) override {}
    };
}

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
%token <yy::LpcType> L_TYPE L_TYPE_MODIFIER
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

%type <uint32_t> storage_or_type typed_storage function_head
%type <uint32_t> type_modifier_list opt_star
%type <std::string> str_literal str_const

%type <std::shared_ptr<AstNode>> program def
%%

/*
 * LPC grammar rules section
 *
 * $$: product of the rule's right-hand side symbols.
 * $n: value of the nth symbol on the right-hand side of the rule.
 *
 * NOTE: The grammar is not from original LPMud source code. It is designed for the
 * LPCVM project with some enhancements and simplifications for this implementation.
 *
 * Use yyget_extra(yyscanner) to access compiler context data (e.g., symbol tables,
 * constant pool, and current program state) in the parser actions.
 *
 * Use std::shared_ptr for dynamically allocated AST nodes to manage memory
 * automatically when exceptions are thrown at parse time.
 *
 * Keep the parser actions concise and focused on AST construction by delegating
 * details to the methods in the AST builder classes. The AstProgramNode represents
 * the root of the AST for the entire program if parsing is successful.
 */

all:
    program { yyget_extra(yyscanner)->prog = $1; }
    ;

program
    : program def extra_semicolon
        { ($$ = $1)->add_child($def); }
    | program inheritance
        { $$ = $1; }
    | %empty
        { $$ = std::make_shared<AstProgramNode>(); }
    ;

extra_semicolon
    : %empty
    | extra_semicolon ';' { /* ignore extra semicolon(s) between definitions */ }
    ;

def
    : function_head
        {
            /* function declaration */
            uint32_t combined_type = $1;
        }
      '(' opt_parameter_list ')'
        {
            /* handle function parameters */
        }
      block_or_semicolon
        {
            /* handle forward declaration or definition for the function */
        }
    | storage_or_type var_list ';'
        {
            /* variable declarations */
            uint32_t combined_type = $1;
        }
    ;

storage_or_type
    : type_modifier_list
        { $$ = $1 | static_cast<uint32_t>(LpcType::T_MIXED); /* implicit variable type: mixed*/ }
    | typed_storage
        { $$ = $1; }
    ;

typed_storage
    : L_TYPE
        { $$ = static_cast<uint32_t>($1); }
    | type_modifier_list L_TYPE
        { $$ = $1 | static_cast<uint32_t>($2); }
    ;

type_modifier_list
    : L_TYPE_MODIFIER
        { $$ = static_cast<uint32_t>($1); }
    | type_modifier_list L_TYPE_MODIFIER
        {
            /* combine the type modifiers using bitwise OR */
            $$ = $1 | static_cast<uint32_t>($2);
        }
    ;

function_head
    : opt_star L_IDENTIFIER
        {
            /* implicit function return type: void */
            $$ = static_cast<uint32_t>(LpcType::T_VOID) | $1;
        }
    | storage_or_type opt_star L_IDENTIFIER
        { $$ = $1 | $2; }
    ;

opt_star
    : %empty
        { $$ = 0; }
    | '*'
        { $$ = static_cast<uint32_t>(LpcType::T_ARRAY); }
    ;

var_list
    : new_var
    | var_list ',' new_var
    ;

new_var
    : opt_star L_IDENTIFIER
    | opt_star L_IDENTIFIER L_ASSIGN expr0
    ;

inheritance
    : L_INHERIT str_const ';'
    | type_modifier_list L_INHERIT str_const ';'
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

opt_parameter_list
    : %empty
    | parameter_list
    ;

parameter_list
    : parameter_decl
    | parameter_list ',' parameter_decl
    | parameter_list L_ELLIPSIS
    ;

parameter_decl
    : L_TYPE opt_star L_IDENTIFIER
    | L_TYPE opt_star { /* anonymous parameter */ }
    | L_IDENTIFIER { /* implicit mixed type parameter */ }
    ;

block_or_semicolon
    : block
    | ';'
    ;

block
    : '{' stmt_list '}'
    ;

stmt_list
    : %empty
    | stmt_list stmt
    ;

stmt
    : comma_expr ';'
    | local_decl
    | if_stmt
    | while_stmt
    | do_stmt
    | return_stmt
    | block
    | /* no-op */ ';'
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

local_decl
    : storage_or_type var_list ';'
    ;

if_stmt
    : L_IF '(' comma_expr ')' stmt optional_else_stmt
    ;

optional_else_stmt
    : %empty %prec LOWER_THAN_ELSE
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
    : L_IDENTIFIER '(' opt_arg_list ')'
    ;

opt_arg_list
    : %empty
    | arg_list
    ;

arg_list
    : expr0
    | arg_list ',' expr0
    ;

%%

// User subroutines section
void yy::LpcParser::error(const std::string &msg) {
    const auto* context = yyget_extra(yyscanner);
    throw std::runtime_error("Line " + std::to_string(yyget_lineno(yyscanner)) + ": " + msg);
}

AstProgramNode::AstProgramNode()
    : AstNode(0, AstNode::Type::Program) {
}
