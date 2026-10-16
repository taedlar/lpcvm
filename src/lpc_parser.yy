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

%token L_INHERIT LAS
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
%type <ASTNode> formal_parameters parameter_list parameter_decl
%type <ASTNode> block_or_semicolon block
%type <ASTNode> stmt_list stmt
%type <ASTNode> local_decl_list var_list new_var
%type <ASTNode> do_stmt for_stmt if_stmt opt_else_stmt return_stmt while_stmt
%type <ASTNode> comma_expr lvalue expr0 expr4
%type <ASTNode> function_call arg_list

%type <SymbolSignature> storage_or_type typed_storage
%type <SymbolSignature> type_modifier_list opt_star

%type <std::shared_ptr<SymbolTable::Entry>> function_head

%type <std::string> str_literal opt_as

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
    : %empty { $$ = AST::Program{}; }
    | program inheritance
        {
            $$ = $1;
            if ($2.node.index() != 0)
                std::get<AST::Program>($$.node).defs.push_back(std::move($2));
        }
    | program def extra_semicolon
        {
            $$ = $1;
            if ($2.node.index() != 0)
                std::get<AST::Program>($$.node).defs.push_back(std::move($2));
        }
    ;

extra_semicolon
    : %empty
    | extra_semicolon ';' { /* ignore extra semicolon(s) between definitions */ }
    ;

def
    : function_head[entry] '('
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($entry->name) && $entry->signature.b.is_const)
                throw parse_error("Function already defined: " + $entry->name, yyscanner);
            lpcc->current_scope->emplace($entry->name, $entry); // add the function to the current scope
            lpcc->current_scope = lpcc->current_scope->new_subscope(); // enter a new subscope for the function body
        }
      formal_parameters[params] ')'
        {
            // add formal parameters to current scope before proceed
            auto lpcc = yyget_extra(yyscanner);
            for (auto& param : std::get<AST::ParameterList>($params.node).parameters) {
                auto entry = std::get<AST::VariableDecl>(param.node).entry;
                if (entry)
                    lpcc->current_scope->emplace(entry->name, entry);
            }
        }
      block_or_semicolon[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope(); // exit the function's subscope
            if ($body.node.index()) {
                $entry->signature.b.is_const = 1; // mark the function name symbol as const (declaration with body)
                auto decl = std::make_unique<ASTNode>(AST::FunctionDecl{
                    $entry,
                    std::make_unique<ASTNode>(std::move($params))
                });
                $$ = AST::FunctionDef{
                    std::move(decl),
                    std::make_unique<ASTNode>(std::move($body))
                 };
                SPDLOG_DEBUG("Added definition of function: {}", $entry->name);
            }
            else {
                // no function body => a forward declaration (allows calling a function before its definition)
                auto lpcc = yyget_extra(yyscanner);
                $$ = AST::FunctionDecl{
                    $entry,
                    std::make_unique<ASTNode>(std::move($params))
                };
                SPDLOG_DEBUG("Forward declaration of function: {}", $entry->name);
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
    ;

typed_storage
    : L_TYPE
        { $$ = SymbolSignature{}; $$.b.data_type = $1; }
    | type_modifier_list L_TYPE
        { ($$ = $1).b.data_type = $2; }
    ;

type_modifier_list
    : L_TYPE_MODIFIER
    | type_modifier_list L_TYPE_MODIFIER
        { ($$ = $1).value |= $2.value; }
    ;

function_head
    : opt_star L_IDENTIFIER
        {
            SymbolSignature sig{0};
            sig.b.data_type = LpcDataType::T_INT; // implicit function return type
            $$ = std::make_shared<SymbolTable::Entry>($2, sig);
        }
    | storage_or_type opt_star L_IDENTIFIER
        {
            $1.value |= $2.value;
            $$ = std::make_shared<SymbolTable::Entry>($3, $1);
        }
    ;

opt_star
    : %empty
        { ($$ = SymbolSignature{}).value = 0; }
    | '*'
        { ($$ = SymbolSignature{}).b.is_array = 1; /* mark as array */ }
    ;

var_list
    : new_var
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
            $$ = AST::VariableDecl{ std::move(entry), nullptr };
        }
    | opt_star L_IDENTIFIER L_ASSIGN expr0
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Variable already defined: " + $2, yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($2, $1);
            lpcc->current_scope->emplace($2, entry); // add the variable to current scope
            $$ = AST::VariableDecl{ std::move(entry), std::make_unique<ASTNode>(std::move($4)) };
        }
    ;

opt_as
    : %empty
        { $$ = ""; }
    | LAS L_IDENTIFIER
        { $$ = $2; }
    ;

inheritance
    : L_INHERIT str_literal opt_as ';'
        {
            auto lpcc = yyget_extra(yyscanner);
            std::string alias{$3.empty() ? $2 : $3};
            if (!lpcc->current_scope->lookup(alias)) {
                auto entry = std::make_shared<SymbolTable::Entry>(alias, SymbolSignature{0}); // alias symbol
                entry->value = $2;
                lpcc->current_scope->emplace(alias, entry); // add the pseudo entry to current scope
                $$ = AST::Inheritance{ entry };
            }
            else if (!$3.empty()) {
                throw parse_error("Alias already defined: " + alias, yyscanner);
            }
        }
    | type_modifier_list L_INHERIT str_literal opt_as ';'
        {
            auto lpcc = yyget_extra(yyscanner);
            std::string alias{$4.empty() ? $3 : $4};
            if (!lpcc->current_scope->lookup(alias)) {
                auto entry = std::make_shared<SymbolTable::Entry>(alias, $1); // alias symbol
                entry->value = $3;
                lpcc->current_scope->emplace(alias, entry); // add the pseudo entry to current scope
                $$ = AST::Inheritance{ entry };
            }
            else if (!$4.empty()) {
                throw parse_error("Alias already defined: " + alias, yyscanner);
            }
        }
    ;

str_literal
    : L_STRING_LITERAL
        { $$ = $1; }
    | str_literal L_STRING_LITERAL
        { $$ = $1 + $2; /* compile-time string concatenation */ }
    ;

formal_parameters
    : %empty
        { $$ = AST::ParameterList{}; }
    | parameter_list
        {
            $$ = AST::ParameterList{};
            std::get<AST::ParameterList>($$.node).parameters = std::move(std::get<AST::List>($1.node).elts);
        }
    ;

parameter_list
    : parameter_decl
        {
            $$ = AST::List{};
            std::get<AST::List>($$.node).elts.push_back(std::move($1));
        }
    | parameter_list ',' parameter_decl
        {
            std::get<AST::List>($1.node).elts.push_back(std::move($3));
            $$ = $1;
        }
    | parameter_list ',' L_ELLIPSIS
        {
            std::get<AST::List>($1.node).elts.push_back(ASTNode{}); // represent ellipsis as an empty ASTNode
            $$ = $1;
        }
    ;

parameter_decl
    : L_TYPE opt_star L_IDENTIFIER
        {
            SymbolSignature s;
            s.b.data_type = $1;
            s.value |= $2.value;
            auto entry = std::make_shared<SymbolTable::Entry>($3, s);
            $$ = AST::VariableDecl{ std::move(entry), nullptr };
        }
    | L_TYPE opt_star
        { $$ = AST::VariableDecl{ nullptr, nullptr }; }
    | L_IDENTIFIER
        {
            auto entry = std::make_shared<SymbolTable::Entry>($1, SymbolSignature{LpcDataType::T_INT});
            $$ = AST::VariableDecl{ std::move(entry), nullptr };
        }
    ;

block_or_semicolon
    : ';'
        { $$ = ASTNode{}; }
    | block
    ;

block
    : '{' stmt_list '}'
        {
            $$ = AST::Block{};
            std::get<AST::Block>($$.node).stmts = std::move(std::get<AST::List>($2.node).elts);
        }
    ;

stmt_list
    : %empty
        { $$ = AST::List{}; /* No-op */ }
    | stmt_list stmt
        {
            if ($2.node.index() != 0)
                std::get<AST::List>($1.node).elts.push_back(std::move($2));
            $$ = $1;
        }
    ;

stmt
    : comma_expr ';'
    | return_stmt
    | local_decl_list
    | do_stmt
    | for_stmt
    | if_stmt
    | while_stmt
    | block
    | L_BREAK ';'
        { $$ = AST::Break{}; }
    | L_CONTINUE ';'
        { $$ = AST::Continue{}; }
    ;

comma_expr
    : expr0
    | comma_expr ',' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    ;

expr0
    : lvalue L_ASSIGN expr0
        { $$ = AST::Assign{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '?' expr0 ':' expr0 %prec '?'
        { $$ = AST::TernaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3)), std::make_unique<ASTNode>(std::move($5))}; }
    | expr0 L_LOR expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 L_LAND expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '|' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '^' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '&' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 L_LSH expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 L_RSH expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '+' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '-' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '*' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '/' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 '%' expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | expr0 L_ORDER expr0
        { $$ = AST::BinaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}; }
    | L_INC lvalue %prec L_NOT
        { $$ = AST::Assign{nullptr, std::make_unique<ASTNode>(std::move($2))}; }
    | L_DEC lvalue %prec L_NOT
        { $$ = AST::Assign{nullptr, std::make_unique<ASTNode>(std::move($2))}; }
    | lvalue L_INC
        { $$ = AST::Assign{std::make_unique<ASTNode>(std::move($1)), nullptr}; }
    | lvalue L_DEC
        { $$ = AST::Assign{std::make_unique<ASTNode>(std::move($1)), nullptr}; }
    | L_NOT expr0
        { $$ = AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))}; }
    | '~' expr0
        { $$ = AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))}; }
    | '-' expr0 %prec L_NOT
        { $$ = AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))}; }
    | expr4
    | L_INTEGER
        { $$ = AST::Constant{ $1 }; }
    | L_REAL_NUMBER
        { $$ = AST::Constant{ $1 }; }    
    | str_literal
        { $$ = AST::StringLiteral{$1, *yyget_extra(yyscanner)->consts}; }
    ;

lvalue
    : expr4
    ;

expr4
    : '(' comma_expr ')'
        { $$ = $2; }
    | L_IDENTIFIER
        {
            auto entry = yyget_extra(yyscanner)->current_scope->lookup(std::string($1));
            if (!entry) {
                // Handle the error, e.g., throw an exception or report an undefined variable
                throw parse_error("Undefined variable: " + std::string($1), yyscanner);
            }
            $$ = AST::Variable{ entry };
        }
    | function_call
    | expr4 '[' comma_expr ']'
    ;

local_decl_list
    : ';'
        { $$ = ASTNode{}; /* i.e. No-op */ }
    | storage_or_type var_list ';'
        { $$ = $2; }
    ;

if_stmt
    : L_IF '(' comma_expr[cond] ')' stmt[body] opt_else_stmt[else_body]
        {
            $$ = AST::Cond{
                std::make_unique<ASTNode>(std::move($cond)),
                std::make_unique<ASTNode>(std::move($body)),
                std::make_unique<ASTNode>(std::move($else_body))
            };
        }
    ;

opt_else_stmt
    : %empty %prec LOWER_THAN_ELSE
        { $$ = ASTNode{}; }
    | L_ELSE stmt
        { $$ = $2;}
    ;

for_stmt
    : L_FOR '(' local_decl_list[init] comma_expr[cond] ';' comma_expr[step] ')' stmt[body]
        {
            ASTNode for_loop = AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move($cond)),
                std::make_unique<ASTNode>(std::move($step)),
                std::make_unique<ASTNode>(std::move($body))
            };
            if ($init.node.index() != 0) {
                std::vector<ASTNode> stmts;
                stmts.push_back(std::move($init));
                stmts.push_back(std::move(for_loop));
                $$ = AST::Block{ std::move(stmts) };
            }
            else {
                $$ = for_loop;
            }
        }
    ;

while_stmt
    : L_WHILE '(' comma_expr[cond] ')' stmt[body]
        {
            $$ = AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move($cond)),
                nullptr,
                std::make_unique<ASTNode>(std::move($body))
            };
        }
    ;

do_stmt
    : L_DO stmt[body] L_WHILE '(' comma_expr[cond] ')' ';'
        {
            $$ = AST::PostTestedLoop{
                std::make_unique<ASTNode>(std::move($body)),
                std::make_unique<ASTNode>(std::move($cond))
            };
        }
    ;

return_stmt
    : L_RETURN ';'
        { $$ = AST::Return{nullptr}; }
    | L_RETURN comma_expr ';'
        { $$ = AST::Return{std::make_unique<ASTNode>(std::move($2))}; }
    ;

function_call
    : L_IDENTIFIER '(' arg_list ')'
        {
            auto lpcc = yyget_extra(yyscanner);
            auto entry = lpcc->current_scope->lookup($1);
            if (!entry) {
                // throw parse_error("Undefined function: " + std::string($1), yyscanner);
                entry = std::make_shared<SymbolTable::Entry>($1, SymbolSignature{0});
                lpcc->current_scope->emplace($1, entry);
            }
            $$ = AST::FunctionCall{ std::move(entry), std::make_unique<ASTNode>(std::move($3)) };
        }
    ;

arg_list
    : %empty
        { $$ = AST::List{}; }
    | expr0
        {
            $$ = AST::List{};
            std::get<AST::List>($$.node).elts.push_back(std::move($1));
        }
    | arg_list ',' expr0
        {
            $$ = $1;
            std::get<AST::List>($$.node).elts.push_back(std::move($3));
        }
    ;

%%

// User subroutines section
void yy::LpcParser::error(const std::string &msg) {
    throw parse_error(msg, yyget_lineno(yyscanner), yyget_column(yyscanner), yyget_extra(yyscanner));
}
