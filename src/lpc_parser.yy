%require "3.2"
%language "C++"
%output "lpc_parser.cpp"
%define api.token.constructor
%define api.value.type variant
%define api.parser.class {LpcParser}
%lex-param { yyscan_t yyscanner }
%parse-param { yyscan_t yyscanner }

%code top {
    #include "config.hpp"
}

%code requires {
    /*
    * SPDX-License-Identifier: MIT
    * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
    */
    #include <cstdint>
    #include <stdexcept>
    #include <string>
    #include <memory>

    #include "symbol_table.hpp"
    #include "ast_node.hpp"
    #include "const_pool.hpp"
    #include "lpc_compiler.hpp"

    namespace yy {
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
            EQUAL = 0,
            NOT_EQUAL,
            LESS,
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

    namespace yy {
        // Exception type for parse errors.
        struct parse_error: std::runtime_error {
            parse_error(const std::string& message, int lineno, int column, YY_EXTRA_TYPE lpcc = nullptr)
                : std::runtime_error(message), lineno(lineno), column(column), lpcc(lpcc) {}
            parse_error(const std::string& message, yyscan_t yyscanner)
                : parse_error(message, yyget_lineno(yyscanner), yyget_column(yyscanner), yyget_extra(yyscanner)) {}
            int lineno;
            int column;
            YY_EXTRA_TYPE lpcc;
        };
    } // namespace yy
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

%token <int> L_TYPE
%token <int> L_ASSIGN L_ORDER

%token <long> L_INTEGER
%token <double> L_REAL_NUMBER

%token <std::string> L_IDENTIFIER
%token <std::string> L_STRING_LITERAL

%token <SymbolSignature> L_TYPE_MODIFIER

// Operator precedence and associativity
%right L_ASSIGN
%right '?'
%left L_LOR
%left L_LAND
%left '|'
%left '^'
%left '&'
%left L_ORDER '<'
%left L_LSH L_RSH
%left '+' '-'
%left '*' '%' '/'
%right L_NOT '~'
%nonassoc L_INC L_DEC

%token L_ELLIPSIS

%type <ASTNode> program def inheritance
%type <ASTNode> stmt_list
%type <ASTNode> local_decl_list var_list new_var
%type <ASTNode> block_or_semicolon block
%type <ASTNode> comma_expr lvalue expr0 expr4
%type <ASTNode> function_call

%type <SymbolSignature> storage_or_type typed_storage
%type <SymbolSignature> type_modifier_list opt_star

%type <SymbolTable::Entry> function_head

%type <std::string> str_literal

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
    program { yyget_extra(yyscanner)->root = std::make_shared<ASTNode>(std::move($1)); }
    ;

program
    : %empty
        { $$ = AST::Program{}; }
    | program inheritance
        {
            $$ = std::move($1);
            if ($2.node.index() != 0)
                std::get<AST::Program>($$.node).defs.push_back(std::move($2));
        }
    | program def extra_semicolon
        {
            $$ = std::move($1);
            if ($2.node.index() != 0)
                std::get<AST::Program>($$.node).defs.push_back(std::move($2));
        }
    ;

extra_semicolon
    : %empty
    | extra_semicolon ';' { /* ignore extra semicolon(s) between definitions */ }
    ;

def
    : function_head '('
        {
            auto lpcc = yyget_extra(yyscanner);
            if (auto se = lpcc->current_scope->lookup($1.name); se && se->signature.b.is_const) {
                throw parse_error("Function already defined: " + $1.name, yyscanner);
            }
            auto entry = std::make_shared<SymbolTable::Entry>($1.name, $1.signature);
            lpcc->current_scope->emplace($1.name, entry); // add the function to the current scope
            lpcc->current_scope = lpcc->current_scope->new_subscope();
        }
      opt_parameter_list[params] ')'
        {
            // TODO: add formal parameters to the symbol table before processing local declarations in the function body
        }
      block_or_semicolon[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            if ($body.node.index() != 0) {
                auto entry = lpcc->current_scope->lookup($1.name);
                entry->signature.b.is_const = 1; // mark the function name symbol as const (declaration with body)
                $$ = AST::FunctionDef{
                    std::make_unique<SymbolTable::Entry>(*entry),
                    std::make_unique<ASTNode>(std::move($body))
                 };
                SPDLOG_DEBUG("Added definition of function: {}", $1.name);
            }
            else {
                // no function body => a forward declaration (allows calling a function before its definition)
                SPDLOG_DEBUG("Forward declaration of function: {}", $1.name);
            }
        }
    | storage_or_type var_list ';'
        {
            /* variable declarations */
        }
    ;

storage_or_type
    : type_modifier_list
        { ($$ = $1).b.data_type = LpcDataType::T_INT; };
    | typed_storage
        { $$ = $1; }
    ;

typed_storage
    : L_TYPE
        { $$.b.data_type = $1; }
    | type_modifier_list L_TYPE
        { $$ = $1; $$.b.data_type = $2; }
    ;

type_modifier_list
    : L_TYPE_MODIFIER
        { $$ = $1; }
    | type_modifier_list L_TYPE_MODIFIER
        { $$.value = $1.value | $2.value; }
    ;

function_head
    : opt_star L_IDENTIFIER
        {
            SymbolSignature sig{0};
            sig.b.data_type = LpcDataType::T_INT; // implicit function return type
            $$ = SymbolTable::Entry{$2, sig};
        }
    | storage_or_type opt_star L_IDENTIFIER
        {
            $1.value |= $2.value;
            $$ = SymbolTable::Entry{$3, $1};
        }
    ;

opt_star
    : %empty
        { $$.value = 0; }
    | '*'
        { $$.b.is_array = 1; }
    ;

var_list
    : new_var
        { $$ = std::move($1); }
    | var_list ',' new_var
        {
            $$ = AST::VariableDeclList{};
            auto& decls = std::get<AST::VariableDeclList>($$.node);
            std::visit (overloaded {
                [&](AST::VariableDecl& decl) { decls.declarations.push_back(std::move($1)); },
                [&](AST::VariableDeclList& decl_list) { decls = std::move(decl_list); },
                [&](auto&) { /* ignore other types */ }
            }, $1.node);
            decls.declarations.push_back(std::move($3));
        }
    ;

new_var
    : opt_star L_IDENTIFIER
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Variable already defined: " + $2, yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($2, $1);
            lpcc->current_scope->emplace($2, entry); // add the variable to current scope
            $$ = ASTNode{ AST::VariableDecl{ entry, nullptr } };
        }
    | opt_star L_IDENTIFIER L_ASSIGN expr0
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Variable already defined: " + $2, yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($2, $1);
            lpcc->current_scope->emplace($2, entry); // add the variable to current scope
            $$ = ASTNode{ AST::VariableDecl{ entry, std::make_unique<ASTNode>(std::move($4)) } };
        }
    ;

inheritance
    : L_INHERIT str_literal ';'
        {
            auto idx = yyget_extra(yyscanner)->consts->find_or_add($2);
            $$ = AST::Inheritance{ idx, nullptr };
        }
    | type_modifier_list L_INHERIT str_literal ';'
        {
            auto idx = yyget_extra(yyscanner)->consts->find_or_add($3);
            $$ = AST::Inheritance{ idx, nullptr };
        }
    ;

str_literal
    : L_STRING_LITERAL
        { $$ = $1; }
    | str_literal L_STRING_LITERAL
        { $$ = $1 + $2; /* compile-time string concatenation */ }
    ;

opt_parameter_list
    : %empty
    | parameter_list
    ;

parameter_list
    : parameter_decl
    | parameter_list ',' parameter_decl
    | parameter_list ',' L_ELLIPSIS
    ;

parameter_decl
    : L_TYPE opt_star L_IDENTIFIER
    | L_TYPE opt_star { /* anonymous parameter */ }
    | L_IDENTIFIER { /* implicit mixed type parameter */ }
    ;

block_or_semicolon
    : ';'
        { $$ = ASTNode{}; }
    | block
        { $$ = ASTNode{ std::move($1) }; }
    ;

block
    : '{' stmt_list '}'
        { $$ = ASTNode{ std::move($2) }; }
    ;

stmt_list
    : %empty
        { $$ = ASTNode{ AST::Block{} }; }
    | stmt_list stmt
        { $$ = ASTNode{ std::move($1) }; }
    ;

stmt
    : comma_expr ';'
    | local_decl_list
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
        { $$ = ASTNode{ std::move($1) }; }
    | comma_expr ',' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    ;

expr0
    : lvalue L_ASSIGN expr0
        { $$ = ASTNode{ AST::Assign{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}}; }
    | expr0 '?' expr0 ':' expr0 %prec '?'
        { $$ = ASTNode{ AST::TernaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3)), std::make_unique<ASTNode>(std::move($5))} }; }
    | expr0 L_LOR expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 L_LAND expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '|' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '^' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '&' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 L_LSH expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 L_RSH expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '+' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '-' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '*' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '/' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 '%' expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | expr0 L_ORDER expr0
        { $$ = ASTNode{ AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))} }; }
    | L_INC lvalue %prec L_NOT
        { $$ = ASTNode{ AST::Assign{nullptr, std::make_unique<ASTNode>(std::move($2))} }; }
    | L_DEC lvalue %prec L_NOT
        { $$ = ASTNode{ AST::Assign{nullptr, std::make_unique<ASTNode>(std::move($2))} }; }
    | lvalue L_INC
        { $$ = ASTNode{ AST::Assign{std::make_unique<ASTNode>(std::move($1)), nullptr} }; }
    | lvalue L_DEC
        { $$ = ASTNode{ AST::Assign{std::make_unique<ASTNode>(std::move($1)), nullptr} }; }
    | L_NOT expr0
        { $$ = ASTNode{ AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))} }; }
    | '~' expr0
        { $$ = ASTNode{ AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))} }; }
    | '-' expr0 %prec L_NOT
        { $$ = ASTNode{ AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))} }; }
    | expr4
        { $$ = ASTNode{ std::move($1) }; }
    | L_INTEGER
        { $$ = ASTNode{ AST::Constant{ $1 } }; }
    | L_REAL_NUMBER
        { $$ = ASTNode{ AST::Constant{ $1 } }; }    
    | str_literal
        { $$ = ASTNode{ AST::StringLiteral{$1, *yyget_extra(yyscanner)->consts} }; }
    ;

lvalue
    : expr4
        { $$ = ASTNode{ std::move($1) }; }
    ;

expr4
    : '(' comma_expr ')'
        { $$ = ASTNode{ std::move($2) }; }
    | L_IDENTIFIER
        {
            auto entry = yyget_extra(yyscanner)->current_scope->lookup(std::string($1));
            if (!entry) {
                // Handle the error, e.g., throw an exception or report an undefined variable
                throw parse_error("Undefined variable: " + std::string($1), yyscanner);
            }
            $$ = ASTNode{ AST::Variable{ entry } };
        }
    | function_call
        { /*$$ = $1;*/ }    
    | expr4 '[' comma_expr ']'
        { $$ = ASTNode{ std::move($1) }; }
    ;

local_decl_list
    : storage_or_type var_list ';'
        { $$ = ASTNode{ std::move($2) }; }
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
    : L_IDENTIFIER '(' arg_list ')'
        {
            $$ = ASTNode{AST::FunctionCall{}};
        }
    ;

arg_list
    : %empty
    | expr0
    | arg_list ',' expr0
    ;

%%

// User subroutines section
void yy::LpcParser::error(const std::string &msg) {
    throw parse_error(msg, yyget_lineno(yyscanner), yyget_column(yyscanner), yyget_extra(yyscanner));
}
