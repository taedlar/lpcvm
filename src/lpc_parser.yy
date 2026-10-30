%require "3.2"
%language "C++"
%output "lpc_parser.cpp"
%define api.token.constructor
%define api.value.type variant
%define api.parser.class {LpcParser}
%locations
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

    #include "vm_opcodes.hpp"
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

    inline ScannerLocation source_location(const yy::location& location, YY_EXTRA_TYPE lpcc) {
        return {
            lpcc ? lpcc->current_file : ConstPool::INVALID_INDEX,
            lpcc ? lpcc->current_func : ConstPool::INVALID_INDEX,
            location.begin.line,
            location.begin.column
        };
    }

    /**
     * Helper macro to assign an AST node (move-assignment assumed) to a target variable.
     * It uses the given arguments and source location to synthesize an AST node.
     * File and function information is retrieved from the compiler context (YY_EXTRA_TYPE).
     * @param target The variable to assign the AST node to.
     * @param location The source location of the AST node as tracked by the Bison parser (yy::location object).
     * @param ... The arguments to construct the AST node.
     */
    #define ASSIGN_AST(target, location, ...) \
        ((target) = ASTNode((__VA_ARGS__), source_location(location, yyget_extra(yyscanner))))

    namespace yy {
        // Exception type for parse errors.
        struct parse_error: std::runtime_error {
            parse_error(const std::string& message, int lineno, int column, YY_EXTRA_TYPE lpcc = nullptr)
                : std::runtime_error(message), lineno(lineno), column(column), lpcc(lpcc) {}
            parse_error(const std::string& message, yyscan_t yyscanner)
                : parse_error(message, yyget_extra(yyscanner)->location.begin.line,
                    yyget_extra(yyscanner)->location.begin.column, yyget_extra(yyscanner)) {}
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
%type <ASTNode> formal_parameters parameter_list
%type <ASTNode> block_or_semicolon block
%type <ASTNode> stmt_list stmt
%type <ASTNode> local_decl_list var_list new_var
%type <ASTNode> do_stmt for_stmt if_stmt opt_else_stmt return_stmt while_stmt
%type <ASTNode> comma_expr lvalue expr0 expr4
%type <ASTNode> function_call arg_list

%type <SymbolSignature> storage_or_type typed_storage parameter_decl
%type <SymbolSignature> type_modifier_list opt_star

%type <std::shared_ptr<SymbolTable::Entry>> function_head

%type <std::string> str_literal opt_as

%%

/*
 * LPC grammar rules section
 *
 * $$: product of the rule's right-hand side symbols.
 * $n: value of the nth symbol on the right-hand side of the rule.
 * @$: The source location of the left-hand side symbol of the rule
 * @n: The source location of the nth symbol on the right-hand side of the rule
 *
 * NOTE: The grammar is not from original LPMud source code. It is designed for the
 * LPCVM project with some enhancements and simplifications for this implementation.
 *
 * Use yyget_extra(yyscanner) to access compiler context data (e.g., symbol tables,
 * constant pool, and current program state) in the parser actions.
 *
 * Use std::shared_ptr for dynamically allocated symbol table entries to manage memory
 * automatically when exceptions are thrown at parse time.
 *
 * Use ASSIGN_AST macro to create and assign AST nodes for target variables with
 * proper source location tracking.
 */

all:
    program { yyget_extra(yyscanner)->root = std::make_shared<ASTNode>(std::move($1)); }
    ;

program
    : %empty { ASSIGN_AST($$, @$, AST::Program{}); }
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
            lpcc->current_func = lpcc->consts->find_or_add($entry->name); // set the current function being parsed
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::FUNCTION, 0); // enter a new subscope for the function body
            lpcc->next_local = 0; // reset overlay base for the function scope
        }
      formal_parameters[params] ')'
        {
            // add formal parameters to current scope before proceed
            auto lpcc = yyget_extra(yyscanner);
            // TODO: number of formal parameters == lpcc->next_local.
            // When pushing a control frame, actual parameters are expected to be on the data stack in order.
            // The number of actual parameters should not exceed the number of formal parameters. (in vararg case,
            // the last formal parameter is an array to catch all actual parameters).
            // If the caller provides fewer actual parameters than the number of formal parameters, the missing
            // parameters are considered uninitialized.
        }
      block_or_semicolon[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            auto func_scope = lpcc->current_scope;
            lpcc->current_scope = func_scope->get_enclosing_scope(); // exit the function's subscope
            lpcc->current_func = ConstPool::INVALID_INDEX; // reset the current function being parsed
            if ($body.node.index()) {
                $entry->signature.b.is_const = 1; // mark the function name symbol as const (has body)
                $entry->value = lpcc->next_func_idx++;
                auto decl = std::make_unique<ASTNode>(AST::FunctionDecl{
                    $entry,
                    std::make_unique<ASTNode>(std::move($params))
                }, source_location(@entry, lpcc));
                ASSIGN_AST($$, @entry, AST::FunctionDef{
                    std::move(decl),
                    std::make_unique<ASTNode>(std::move($body))
                });
                SPDLOG_DEBUG("Added definition of function: {}", $entry->name);
            }
            else {
                // no function body => a forward declaration (allows calling a function before its definition)
                auto lpcc = yyget_extra(yyscanner);
                ASSIGN_AST($$, @entry, AST::FunctionDecl{
                    $entry,
                    std::make_unique<ASTNode>(std::move($params))
                });
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
            sig.b.is_function = 1; // mark as function
            $$ = std::make_shared<SymbolTable::Entry>($2, sig);
            if (!$1.b.is_array)
                @$.begin = @2.begin;
        }
    | storage_or_type opt_star L_IDENTIFIER
        {
            $1.value |= $2.value;
            $1.b.is_function = 1; // mark as function
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
            if ($3.node.index() == 0) {
                $$ = $1;
            }
            else if ($1.node.index() == 0) {
                $$ = $3;
            }
            else {
                ASSIGN_AST($$, @1, AST::BinaryOp{
                    oparg::COMMA,
                    std::make_unique<ASTNode>(std::move($1)),
                    std::make_unique<ASTNode>(std::move($3))
                });
            }
        }
    ;

new_var
    : opt_star L_IDENTIFIER
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Variable already defined: " + $2, yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($2, $1);
            if (lpcc->current_scope->get_enclosing_scope()) { // LOCAL or BLOCK scope
                entry->value = uint32_t{lpcc->next_local++};
                entry->signature.b.is_local = 1; // mark as local variable
            }
            else
                entry->value = uint32_t{lpcc->next_global++};
            lpcc->current_scope->emplace($2, entry); // add the variable to current scope
            ASSIGN_AST($$, @2, std::monostate{}); // no-op
            if (!$1.b.is_array)
                @$.begin = @2.begin;
        }
    | opt_star L_IDENTIFIER L_ASSIGN expr0
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Variable already defined: " + $2, yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($2, $1);
            if (lpcc->current_scope->get_enclosing_scope()) { // LOCAL or BLOCK scope
                entry->value = uint32_t{lpcc->next_local++};
                entry->signature.b.is_local = 1; // mark as local variable
            }
            else
                entry->value = uint32_t{lpcc->next_global++};
            lpcc->current_scope->emplace($2, entry); // add the variable to current scope
            ASSIGN_AST($$, @2, AST::BinaryOp{
                oparg::ASSIGN,
                std::make_unique<ASTNode>(AST::Variable{ std::move(entry) }, source_location(@2, lpcc)), // lhs
                std::make_unique<ASTNode>(std::move($4))
            });
            if (!$1.b.is_array)
                @$.begin = @2.begin;
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
                ASSIGN_AST($$, @$, AST::Inheritance{ entry });
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
                ASSIGN_AST($$, @$, AST::Inheritance{ entry });
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
        { ASSIGN_AST($$, @$, AST::ParameterList{}); }
    | parameter_list
    ;

parameter_list
    : parameter_decl
        {
            ASSIGN_AST($$, @$, AST::ParameterList{});
            std::get<AST::ParameterList>($$.node).parameters.push_back(std::move($1));
        }
    | parameter_list ',' parameter_decl
        {
            auto& params = std::get<AST::ParameterList>($1.node).parameters;
            if(params.back().b.data_type == LpcDataType::T_VOID)
                throw parse_error("Cannot have parameters after a void parameter", yyscanner);
            if(params.back().b.data_type == LpcDataType::T_VARARG)
                throw parse_error("Cannot have parameters after a vararg parameter", yyscanner);
            params.push_back(std::move($3));
            $$ = $1;
        }
    ;

parameter_decl
    : L_TYPE opt_star L_IDENTIFIER
        {
            SymbolSignature s = $2;
            s.b.data_type = $1;
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($3))
                throw parse_error("Parameter name already defined: " + std::string($3), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($3, s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace($3, entry);
            $$ = s;
        }
    | L_TYPE opt_star
        {
            SymbolSignature s = $2;
            s.b.data_type = $1;
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local++; // increment local variable counter for unnamed parameter
            $$ = s;
        }
    | L_IDENTIFIER
        {
            SymbolSignature s{LpcDataType::T_INT};
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($1))
                throw parse_error("Parameter name already defined: " + std::string($1), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($1, s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace($1, entry);
            $$ = s;
        }
    | L_ELLIPSIS L_IDENTIFIER
        {
            SymbolSignature s{LpcDataType::T_VARARG}; // vararg parameter, a catch-all array for remaining actual parameters
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Parameter name already defined: " + std::string($2), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($2, s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace($2, entry);
            $$ = s;
        }
    ;

block_or_semicolon
    : ';'
        { ASSIGN_AST($$, @$, std::monostate{}); }
    | block
    ;

block
    : '{'
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::FUNCTION) {
                // The lpcc->current_scope contains formal parameters of the function.
                // Now entering the local scope for the function body.
                lpcc->current_scope->set_scope_type(SymbolTable::ScopeType::LOCAL);
            }
            else if (lpcc->current_scope->scope_type() != SymbolTable::ScopeType::GLOBAL) {
                lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::BLOCK, lpcc->next_local);
            }
        }
     stmt_list '}'
        {
            auto lpcc = yyget_extra(yyscanner);
            auto num_locals = lpcc->next_local;
            if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::BLOCK) {
                num_locals -= lpcc->current_scope->overlay_base();
                lpcc->next_local = lpcc->current_scope->overlay_base(); // restore the local variable counter to the overlay base of the enclosing block
                lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            }
            else if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::LOCAL) {
                lpcc->next_local = 0;
            }
            ASSIGN_AST($$, @$, AST::Block{
                .num_locals = num_locals,
                .stmts = std::make_unique<ASTNode>(std::move($3))
            });
        }
    ;

stmt_list
    : %empty
        { ASSIGN_AST($$, @$, AST::List{}); /* No-op */ }
    | stmt_list stmt
        {
            if ($2.node.index() != 0) {
                if (std::get<AST::List>($1.node).elts.empty())
                    $1.src = $2.src;
                std::get<AST::List>($1.node).elts.push_back(std::move($2));
            }
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
        { ASSIGN_AST($$, @$, AST::Break{}); }
    | L_CONTINUE ';'
        { ASSIGN_AST($$, @$, AST::Continue{}); }
    ;

comma_expr
    : expr0
    | comma_expr ',' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::COMMA, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    ;

expr0
    : lvalue L_ASSIGN expr0
        { ASSIGN_AST($$, @$, AST::Assign{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '?' expr0 ':' expr0 %prec '?'
        { ASSIGN_AST($$, @$, AST::TernaryOp{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3)), std::make_unique<ASTNode>(std::move($5))}); }
    | expr0 L_LOR expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 L_LAND expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '|' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '^' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '&' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 L_LSH expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 L_RSH expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '+' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::ADD, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '-' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::SUB, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '*' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::MUL, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '/' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::DIV, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '%' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::MOD, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 L_ORDER expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{0, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | L_INC lvalue %prec L_NOT
        { ASSIGN_AST($$, @$, AST::Assign{nullptr, std::make_unique<ASTNode>(std::move($2))}); }
    | L_DEC lvalue %prec L_NOT
        { ASSIGN_AST($$, @$, AST::Assign{nullptr, std::make_unique<ASTNode>(std::move($2))}); }
    | lvalue L_INC
        { ASSIGN_AST($$, @$, AST::Assign{std::make_unique<ASTNode>(std::move($1)), nullptr}); }
    | lvalue L_DEC
        { ASSIGN_AST($$, @$, AST::Assign{std::make_unique<ASTNode>(std::move($1)), nullptr}); }
    | L_NOT expr0
        { ASSIGN_AST($$, @$, AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))}); }
    | '~' expr0
        { ASSIGN_AST($$, @$, AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))}); }
    | '-' expr0 %prec L_NOT
        { ASSIGN_AST($$, @$, AST::UnaryOp{std::make_unique<ASTNode>(std::move($2))}); }
    | expr4
    | L_INTEGER
        {
            auto consts = yyget_extra(yyscanner)->consts;
            if ($1 > 0) {
                // determine if $1 fits in an unsigned 32-bit integer
                if ($1 > std::numeric_limits<uint32_t>::min() && $1 < std::numeric_limits<uint32_t>::max()) {
                    uint32_t u32 = static_cast<uint32_t>($1);
                    ASSIGN_AST($$, @$, AST::NumericConst{ u32, consts->find_or_add(u32) });
                }
                else
                    ASSIGN_AST($$, @$, AST::NumericConst{ $1, consts->find_or_add($1) });
            }
            else {
                // determine if $1 fits in a signed 32-bit integer
                if ($1 > std::numeric_limits<int32_t>::min() && $1 < std::numeric_limits<int32_t>::max()) {
                    int32_t i32 = static_cast<int32_t>($1);
                    ASSIGN_AST($$, @$, AST::NumericConst{ i32, consts->find_or_add(i32) });
                }
                else
                        ASSIGN_AST($$, @$, AST::NumericConst{ $1, consts->find_or_add($1) });
            }
        }
    | L_REAL_NUMBER
        {
            auto consts = yyget_extra(yyscanner)->consts;
            ASSIGN_AST($$, @$, AST::NumericConst{ $1, consts->find_or_add($1) });
        }    
    | str_literal
        { ASSIGN_AST($$, @$, AST::StringLiteral{$1, *yyget_extra(yyscanner)->consts}); }
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
            ASSIGN_AST($$, @$, AST::Variable{ entry });
        }
    | function_call
    | expr4 '[' comma_expr ']'
    ;

local_decl_list
    : ';'
        { ASSIGN_AST($$, @$, std::monostate{}); /* i.e. No-op */ }
    | storage_or_type var_list ';'
        { $$ = $2; }
    ;

if_stmt
    : L_IF '(' comma_expr[cond] ')' stmt[body] opt_else_stmt[else_body]
        {
            ASSIGN_AST($$, @$, AST::Cond{
                std::make_unique<ASTNode>(std::move($cond)),
                std::make_unique<ASTNode>(std::move($body)),
                std::make_unique<ASTNode>(std::move($else_body))
            });
        }
    ;

opt_else_stmt
    : %empty %prec LOWER_THAN_ELSE
        { ASSIGN_AST($$, @$, std::monostate{}); }
    | L_ELSE stmt
        { $$ = $2;}
    ;

for_stmt
    : L_FOR '(' local_decl_list[init] comma_expr[cond] ';' comma_expr[step] ')' stmt[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            ASTNode for_loop;
            ASSIGN_AST(for_loop, @$, AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move($cond)),
                std::make_unique<ASTNode>(std::move($step)),
                std::make_unique<ASTNode>(std::move($body))
            });
            if ($init.node.index() != 0) {
                std::vector<ASTNode> stmts;
                auto list_source = $init.src;
                stmts.push_back(std::move($init));
                stmts.push_back(std::move(for_loop));
                ASSIGN_AST($$, @$, AST::Block{ lpcc->next_local, std::make_unique<ASTNode>(AST::List{ std::move(stmts) }, list_source) });
            }
            else {
                $$ = for_loop;
            }
        }
    ;

while_stmt
    : L_WHILE '(' comma_expr[cond] ')' stmt[body]
        {
            ASSIGN_AST($$, @$, AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move($cond)),
                nullptr,
                std::make_unique<ASTNode>(std::move($body))
            });
        }
    ;

do_stmt
    : L_DO stmt[body] L_WHILE '(' comma_expr[cond] ')' ';'
        {
            ASSIGN_AST($$, @$, AST::PostTestedLoop{
                std::make_unique<ASTNode>(std::move($body)),
                std::make_unique<ASTNode>(std::move($cond))
            });
        }
    ;

return_stmt
    : L_RETURN ';'
        { ASSIGN_AST($$, @$, AST::Return{nullptr}); }
    | L_RETURN comma_expr ';'
        { ASSIGN_AST($$, @$, AST::Return{std::make_unique<ASTNode>(std::move($2))}); }
    ;

function_call
    : L_IDENTIFIER '(' arg_list ')'
        {
            auto lpcc = yyget_extra(yyscanner);
            auto entry = lpcc->current_scope->lookup($1);
            if (!entry) {
                // add forward declaration for the function
                SymbolSignature sig{};
                sig.b.is_function = 1; // mark as function
                entry = std::make_shared<SymbolTable::Entry>($1, sig);
                lpcc->current_scope->emplace($1, entry);
            }
            entry->signature.b.referenced = 1; // mark the function as referenced
            ASSIGN_AST($$, @$, AST::FunctionCall{ std::move(entry), std::make_unique<ASTNode>(std::move($3)) });
        }
    ;

arg_list
    : %empty
        { ASSIGN_AST($$, @$, AST::List{}); }
    | expr0
        {
            ASSIGN_AST($$, @$, AST::List{});
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
void yy::LpcParser::error(const location_type& location, const std::string &msg) {
    throw parse_error(msg, location.begin.line, location.begin.column, yyget_extra(yyscanner));
}
