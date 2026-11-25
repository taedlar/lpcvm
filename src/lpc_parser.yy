%require "3.2"
%language "C++"
%output "lpc_parser.cpp"
%define api.token.constructor
%define api.value.type variant
%define api.parser.class {LpcParser}
%locations
%expect 0
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
    extern YY_EXTRA_TYPE yyget_extra(yyscan_t yyscanner);

    extern int yyget_lineno (yyscan_t yyscanner);
    extern int yyget_column (yyscan_t yyscanner);

    inline ScannerLocation source_location(const yy::location& location, YY_EXTRA_TYPE lpcc) {
        return {
            lpcc && lpcc->consts && location.begin.filename
                ? lpcc->consts->find_or_add(*location.begin.filename)
                : (lpcc ? lpcc->current_file : ConstPool::INVALID_INDEX),
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
        // exception type for parse errors.
        struct parse_error: std::runtime_error {
            parse_error(const std::string& message, int lineno, int column, YY_EXTRA_TYPE lpcc = nullptr)
                : std::runtime_error(message), lineno(lineno), column(column), lpcc(lpcc),
                  filename(lpcc && lpcc->location.begin.filename ? *lpcc->location.begin.filename : "") {}
            parse_error(const std::string& message, const yy::location& location, YY_EXTRA_TYPE lpcc)
                : parse_error(message, location.begin.line, location.begin.column, lpcc) {
                filename = location.begin.filename ? *location.begin.filename : "";
            }
            parse_error(const std::string& message, yyscan_t yyscanner)
                : parse_error(message, yyget_extra(yyscanner)->location.begin.line,
                    yyget_extra(yyscanner)->location.begin.column, yyget_extra(yyscanner)) {}
            int lineno;
            int column;
            YY_EXTRA_TYPE lpcc;
            std::string filename;
        };
    } // namespace yy
}

%token L_MOUNT L_INHERIT L_USING
%token L_IF L_ELSE
%token L_WHILE L_DO L_FOR L_FOREACH L_IN
%token L_BREAK L_CONTINUE
%token L_RETURN
%token L_SWITCH L_CASE L_DEFAULT
%token L_CATCH L_SELF
%token L_NEW L_DESTRUCT

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
// In an argument list, bare name = value is a keyword argument.
%precedence POSITIONAL_IDENTIFIER
%right L_ASSIGN '='
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

%token L_ARRAY_OPEN L_MAPPING_OPEN
%token L_ARROW L_FAT_ARROW
%token L_COLON_COLON
%token L_ELLIPSIS

%type <ASTNode> program def mount inheritance inheritance_decl
%type <ASTNode> formal_parameters parameter_list res_declare res_alias res_decl_list res_acquire
%type <ASTNode> block_or_semicolon block
%type <ASTNode> stmt_list stmt
%type <ASTNode> local_decl_list var_list new_var loop_init opt_comma_expr
%type <ASTNode> do_stmt for_stmt foreach_stmt if_stmt opt_else_stmt return_stmt while_stmt
%type <ASTNode> switch_stmt destruct_stmt
%type <ASTNode> expr_or_block const_expr comma_expr lvalue expr0 expr4
%type <ASTNode> aggregation expr_list assoc_list
%type <ASTNode> catch_clause function_call arg_list arguments argument kwarg
%type <ASTNode> opt_init_list

%type <SymbolSignature> storage_or_type typed_storage
%type <SymbolSignature> type_modifier_list opt_star

%type <std::shared_ptr<SymbolTable::Entry>> function_head new_param defined_id

%type <std::string> str_literal new_res
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
    : mount
    | inheritance
    | function_head[entry] '('
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($entry->name) && $entry->signature.b.is_const)
                throw parse_error("Function already defined: " + $entry->name, yyscanner);
            lpcc->current_scope->emplace($entry->name, $entry); // add the function to the current scope
            lpcc->current_func = lpcc->consts->find_or_add($entry->name); // set the current function being parsed
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::FUNCTION, 0, lpcc->next_local_mount); // enter a new subscope for the function body
            lpcc->next_local = 0; // reset overlay base for the function scope
            lpcc->next_local_mount = 0;
            lpcc->max_local_mounts = 0;
        }
      formal_parameters[params] ')' block_or_semicolon[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            auto func_scope = lpcc->current_scope;
            lpcc->next_local_mount = func_scope->mount_overlay_base();
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
                    std::make_unique<ASTNode>(std::move($body)),
                    lpcc->max_local_mounts
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
            $$ = $2;
        }
    ;

storage_or_type
    : type_modifier_list
        {
            ($$ = $1).b.data_type = LpcDataType::T_INT;
            yyget_extra(yyscanner)->variable_signature = $$;
        }
    | typed_storage
        {
            $$ = $1;
            yyget_extra(yyscanner)->variable_signature = $$;
        }
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
            auto signature = lpcc->variable_signature;
            signature.value |= $1.value;
            auto entry = std::make_shared<SymbolTable::Entry>($2, signature);
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
    | opt_star L_IDENTIFIER '=' expr0
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Variable already defined: " + $2, yyscanner);
            auto signature = lpcc->variable_signature;
            signature.value |= $1.value;
            auto entry = std::make_shared<SymbolTable::Entry>($2, signature);
            if (lpcc->current_scope->get_enclosing_scope()) { // LOCAL or BLOCK scope
                entry->value = uint32_t{lpcc->next_local++};
                entry->signature.b.is_local = 1; // mark as local variable
            }
            else
                entry->value = uint32_t{lpcc->next_global++};
            lpcc->current_scope->emplace($2, entry); // add the variable to current scope
            ASSIGN_AST($$, @2, AST::Assign{
                oparg::ASSIGN,
                std::make_unique<ASTNode>(AST::Variable{ std::move(entry) }, source_location(@2, lpcc)), // lhs
                std::make_unique<ASTNode>(std::move($4))
            });
            if (!$1.b.is_array)
                @$.begin = @2.begin;
        }
    ;

mount
    : L_MOUNT '(' res_decl_list ')' stmt opt_else_stmt
        {
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::GLOBAL) {
                for (auto& declaration : std::get<AST::List>($3.node).elts) {
                    auto* resource = std::get_if<AST::ResourceDecl>(&declaration.node);
                    if (!resource || std::holds_alternative<AST::StringLiteral>(resource->path->node))
                        continue;
                    // Fold literal string concatenations, without evaluating runtime expressions.
                    const auto constants = lpcc->consts->freeze();
                    std::string path;
                    std::vector<const ASTNode*> pending{resource->path.get()};
                    while (!pending.empty()) {
                        const auto* part = pending.back();
                        pending.pop_back();
                        if (const auto* literal = std::get_if<AST::StringLiteral>(&part->node)) {
                            path.append(constants->fetch_string(literal->idx)->view());
                        } else if (const auto* binary = std::get_if<AST::BinaryOp>(&part->node);
                                   binary && binary->arg == oparg::ADD) {
                            pending.push_back(binary->rhs.get());
                            pending.push_back(binary->lhs.get());
                        } else {
                            throw parse_error("Global mount path must be a compile-time string",
                                              part->src.lineno, part->src.column, lpcc);
                        }
                    }
                    resource->path->node = AST::StringLiteral{path, *lpcc->consts};
                }
            }
            ASSIGN_AST($$, @$, AST::Mount{
                std::make_unique<ASTNode>(std::move($3)),
                std::make_unique<ASTNode>(std::move($5)),
                std::make_unique<ASTNode>(std::move($6))
            });
        }
    ;

new_res
    : L_IDENTIFIER
    | new_res L_COLON_COLON L_IDENTIFIER
        { $$ = $1 + "::" + $3; }
    ;

res_declare
    : str_literal
        {
            auto lpcc = yyget_extra(yyscanner);
            ASSIGN_AST($$, @$, AST::StringLiteral{ $1, *lpcc->consts });
        }
    | res_alias
    ;

res_alias
    : expr0[path] L_FAT_ARROW new_res[alias]
        {
            auto lpcc = yyget_extra(yyscanner);
            if (!lpcc->current_scope->lookup($alias)) {
                auto entry = std::make_shared<SymbolTable::Entry>($alias, SymbolSignature{0}); // alias symbol
                entry->signature.b.data_type = T_RESOURCE;
                if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::GLOBAL) {
                    entry->value = lpcc->next_mount++;
                } else {
                    entry->signature.b.is_local = 1;
                    entry->value = lpcc->next_local_mount++;
                    if (lpcc->next_local_mount > lpcc->max_local_mounts)
                        lpcc->max_local_mounts = lpcc->next_local_mount;
                }
                lpcc->current_scope->emplace($alias, entry); // add the pseudo entry to current scope
                ASSIGN_AST($$, @$, AST::ResourceDecl{ entry, std::make_unique<ASTNode>(std::move($path)) });
            }
            else {
                throw parse_error("Resource descriptor already defined: " + $alias, yyscanner);
            }
        }
    ;

res_decl_list
    : res_declare
        {
            ASSIGN_AST($$, @$, AST::List{});
            std::get<AST::List>($$.node).elts.push_back(std::move($1));
        }
    | res_decl_list ',' res_declare
        {
            auto elts = std::move(std::get<AST::List>($1.node).elts);
            elts.push_back(std::move($3));
            ASSIGN_AST($$, @$, AST::List{ std::move(elts) });
        }
    | res_decl_list ','
    ;

inheritance
    : inheritance_decl
    | type_modifier_list inheritance_decl
        {
            auto inheritance = std::move(std::get<AST::Inheritance>($2.node));
            inheritance.signature.value |= $1.value;
            if (auto* resource = std::get_if<AST::ResourceDecl>(&inheritance.res_decl->node))
                resource->entry->signature.value |= $1.value;
            ASSIGN_AST($$, @$, std::move(inheritance));
        }
    ;

inheritance_decl
    : L_INHERIT res_declare ';'
        {
            ASSIGN_AST($$, @$, AST::Inheritance{ std::make_unique<ASTNode>(std::move($2)) });
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
    : new_param
        { ASSIGN_AST($$, @$, AST::ParameterList{ std::vector<std::shared_ptr<SymbolTable::Entry>>{std::move($1)} }); }
    | parameter_list ',' new_param
        {
            auto& params = std::get<AST::ParameterList>($1.node).parameters;
            if(params.back()->signature.b.data_type == LpcDataType::T_VOID)
                throw parse_error("Cannot have parameters after a void parameter", yyscanner);
            if(params.back()->signature.b.data_type == LpcDataType::T_VARARG)
                throw parse_error("Cannot have parameters after a vararg parameter", yyscanner);
            if(params.size() == 255)
                throw parse_error("Cannot have more than 255 parameters", yyscanner);
            params.push_back(std::move($3));
            $$ = $1;
        }
    ;

new_param
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
            $$ = entry;
        }
    | L_TYPE opt_star
        {
            SymbolSignature s = $2;
            s.b.data_type = $1;
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local++; // increment local variable counter for anonymous parameter
            $$ = std::make_shared<SymbolTable::Entry>("", s);
        }
    | L_IDENTIFIER
        {
            SymbolSignature s{LpcDataType::T_INT}; // default type for untyped parameter
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($1))
                throw parse_error("Parameter name already defined: " + std::string($1), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($1, s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace($1, entry);
            $$ = entry;
        }
    | L_ELLIPSIS L_IDENTIFIER
        {
            SymbolSignature s{LpcDataType::T_VARARG}; // catch-all mapping for remaining actual parameters
            auto lpcc = yyget_extra(yyscanner);
            if (lpcc->current_scope->lookup($2))
                throw parse_error("Parameter name already defined: " + std::string($2), yyscanner);
            auto entry = std::make_shared<SymbolTable::Entry>($2, s);
            entry->signature.b.is_local = 1;
            entry->value = uint32_t{lpcc->next_local++};
            lpcc->current_scope->emplace($2, entry);
            $$ = entry;
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
                lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::BLOCK, lpcc->next_local, lpcc->next_local_mount);
            }
        }
     stmt_list '}'
        {
            auto lpcc = yyget_extra(yyscanner);
            auto num_locals = lpcc->next_local;
            if (lpcc->current_scope->scope_type() == SymbolTable::ScopeType::BLOCK) {
                num_locals -= lpcc->current_scope->overlay_base();
                lpcc->next_local = lpcc->current_scope->overlay_base(); // restore the local variable counter to the overlay base of the enclosing block
                lpcc->next_local_mount = lpcc->current_scope->mount_overlay_base();
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
    : label stmt
        { $$ = $2; }
    | comma_expr ';'
    | L_USING '(' expr4 ')' stmt[body]
        { $$ = $body; }
    | mount
    | return_stmt
    | local_decl_list
    | destruct_stmt
    | do_stmt
    | for_stmt
    | foreach_stmt
    | if_stmt
    | while_stmt
    | switch_stmt
    | block
    | L_BREAK ';'
        { ASSIGN_AST($$, @$, AST::Break{}); }
    | L_CONTINUE ';'
        { ASSIGN_AST($$, @$, AST::Continue{}); }
    ;

// Each recursive labeled statement must consume a label.
label
    : L_CASE const_expr ':'
        {
            if (yyget_extra(yyscanner)->switch_depth == 0)
                throw parse_error("case label outside switch", @1, yyget_extra(yyscanner));
        }
    | L_DEFAULT ':'
        {
            if (yyget_extra(yyscanner)->switch_depth == 0)
                throw parse_error("default label outside switch", @1, yyget_extra(yyscanner));
        }
    ;

const_expr
    : L_INTEGER
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

comma_expr
    : expr0
    | comma_expr ',' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::COMMA, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    ;

expr0
    : lvalue L_ASSIGN expr0
        { ASSIGN_AST($$, @$, AST::Assign{$2, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | lvalue '=' expr0
        { ASSIGN_AST($$, @$, AST::Assign{oparg::ASSIGN, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | L_INC lvalue %prec L_NOT
        { ASSIGN_AST($$, @$, AST::Assign{oparg::ASSIGN_PREFIX_INC, nullptr, std::make_unique<ASTNode>(std::move($2))}); }
    | L_DEC lvalue %prec L_NOT
        { ASSIGN_AST($$, @$, AST::Assign{oparg::ASSIGN_PREFIX_DEC, nullptr, std::make_unique<ASTNode>(std::move($2))}); }
    | lvalue L_INC
        { ASSIGN_AST($$, @$, AST::Assign{oparg::ASSIGN_SUFFIX_INC, std::make_unique<ASTNode>(std::move($1)), nullptr}); }
    | lvalue L_DEC
        { ASSIGN_AST($$, @$, AST::Assign{oparg::ASSIGN_SUFFIX_DEC, std::make_unique<ASTNode>(std::move($1)), nullptr}); }
    | expr0 '?' expr0 ':' expr0 %prec '?'
        {
            ASSIGN_AST($$, @$, AST::TernaryOp{
                std::make_unique<ASTNode>(std::move($1)),
                std::make_unique<ASTNode>(std::move($3)),
                std::make_unique<ASTNode>(std::move($5))}
            );
        }
    | expr0 L_LOR expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::LOGICAL_OR, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 L_LAND expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::LOGICAL_AND, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '|' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::BITWISE_OR, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '^' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::BITWISE_XOR, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 '&' expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::BITWISE_AND, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 L_LSH expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::LSH, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | expr0 L_RSH expr0
        { ASSIGN_AST($$, @$, AST::BinaryOp{oparg::RSH, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
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
        { ASSIGN_AST($$, @$, AST::BinaryOp{$2, std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | L_NOT expr0
        { ASSIGN_AST($$, @$, AST::UnaryOp{oparg::NOT, std::make_unique<ASTNode>(std::move($2))}); }
    | '~' expr0
        { ASSIGN_AST($$, @$, AST::UnaryOp{oparg::BITWISE_NOT, std::make_unique<ASTNode>(std::move($2))}); }
    | '-' expr0 %prec L_NOT
        { ASSIGN_AST($$, @$, AST::UnaryOp{oparg::NEG, std::make_unique<ASTNode>(std::move($2))}); }
    | expr4
    | const_expr
    | aggregation
    ;

// Assignment targets are primary expressions; assignments themselves are expressions.
lvalue
    : expr4
    ;

expr4
    : '(' comma_expr ')'
        { $$ = $2; }
    | L_SELF
        { ASSIGN_AST($$, @$, AST::Self{ }); }
    | defined_id
        { ASSIGN_AST($$, @$, AST::Variable{ $1 }); }
    | expr4 '[' comma_expr ']'
        { ASSIGN_AST($$, @$, AST::Index{std::make_unique<ASTNode>(std::move($1)), std::make_unique<ASTNode>(std::move($3))}); }
    | function_call
    | catch_clause
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

loop_init
    : local_decl_list
    | comma_expr ';'
    ;

opt_comma_expr
    : %empty
        { ASSIGN_AST($$, @$, std::monostate{}); }
    | comma_expr
    ;

for_begin
    : L_FOR '('
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local, lpcc->next_local_mount);
        }
    ;

for_stmt
    : for_begin loop_init[init] opt_comma_expr[cond] ';' opt_comma_expr[step] ')' stmt[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            // leave the loop scope, releasing the locals declared in the init clause
            const uint32_t num_locals = lpcc->next_local - lpcc->current_scope->overlay_base();
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->next_local_mount = lpcc->current_scope->mount_overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
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
                ASSIGN_AST($$, @$, AST::Block{ num_locals, std::make_unique<ASTNode>(AST::List{ std::move(stmts) }, list_source) });
            }
            else {
                $$ = for_loop;
            }
        }
    ;

foreach_begin
    : L_FOREACH '('
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local, lpcc->next_local_mount);
        }
    ;

foreach_stmt
    : foreach_begin comma_expr[iter] L_IN expr4[collection] ')' stmt[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->next_local_mount = lpcc->current_scope->mount_overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            ASSIGN_AST($$, @$, AST::IterativeLoop{
                std::make_unique<ASTNode>(std::move($iter)),
                std::make_unique<ASTNode>(std::move($collection)),
                std::make_unique<ASTNode>(std::move($body))
            });
        }
    ;

while_begin
    : L_WHILE '('
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->current_scope = lpcc->current_scope->new_subscope(SymbolTable::ScopeType::LOOP, lpcc->next_local, lpcc->next_local_mount);
        }
    ;

while_stmt
    : while_begin comma_expr[cond] ')' stmt[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->next_local_mount = lpcc->current_scope->mount_overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            ASSIGN_AST($$, @$, AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move($cond)),
                nullptr,
                std::make_unique<ASTNode>(std::move($body))
            });
        }
    | while_begin loop_init[init] comma_expr[cond] ')' stmt[body]
        {
            auto lpcc = yyget_extra(yyscanner);
            // leave the loop scope, releasing the locals declared in the init clause
            const uint32_t num_locals = lpcc->next_local - lpcc->current_scope->overlay_base();
            lpcc->next_local = lpcc->current_scope->overlay_base();
            lpcc->next_local_mount = lpcc->current_scope->mount_overlay_base();
            lpcc->current_scope = lpcc->current_scope->get_enclosing_scope();
            ASTNode loop;
            ASSIGN_AST(loop, @$, AST::PreTestedLoop{
                std::make_unique<ASTNode>(std::move($cond)),
                nullptr,
                std::make_unique<ASTNode>(std::move($body))
            });
            if ($init.node.index() != 0) {
                std::vector<ASTNode> stmts;
                auto list_source = $init.src;
                stmts.push_back(std::move($init));
                stmts.push_back(std::move(loop));
                ASSIGN_AST($$, @$, AST::Block{ num_locals, std::make_unique<ASTNode>(AST::List{ std::move(stmts) }, list_source) });
            }
            else {
                $$ = loop;
            }
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

switch_stmt
    : L_SWITCH '(' comma_expr[cond] ')'
        { ++yyget_extra(yyscanner)->switch_depth; }
      stmt[body]
        {
            --yyget_extra(yyscanner)->switch_depth;
            ASSIGN_AST($$, @$, AST::Switch{
                std::make_unique<ASTNode>(std::move($cond)),
                std::make_unique<ASTNode>(std::move($body))
            });
        }
    ;

expr_or_block
    : '(' comma_expr ')'
        { $$ = std::move($2); }
    | block
        { $$ = std::move($1); }
    ;

catch_clause
    : L_CATCH expr_or_block
        { ASSIGN_AST($$, @$, AST::Catch{ std::make_unique<ASTNode>(std::move($2)) }); }
    ;

destruct_stmt
    : L_DESTRUCT lvalue ';'
        {
            ASSIGN_AST($$, @$, AST::Destruct{
                std::make_unique<ASTNode>(std::move($2))
            });
        }
    ;

return_stmt
    : L_RETURN ';'
        { ASSIGN_AST($$, @$, AST::Return{nullptr}); }
    | L_RETURN comma_expr ';'
        { ASSIGN_AST($$, @$, AST::Return{std::make_unique<ASTNode>(std::move($2))}); }
    ;

expr_list
    : expr0
        {
            ASSIGN_AST($$, @$, AST::List{});
            std::get<AST::List>($$.node).elts.push_back(std::move($1));
        }
    | expr_list ',' expr0
        {
            auto &elts = std::get<AST::List>($1.node).elts;
            elts.push_back(std::move($3));
            $$ = $1;
        }
    ;

assoc_list
    : expr0 ':' expr0
        {
            ASSIGN_AST($$, @$, AST::List{});
            auto& elts = std::get<AST::List>($$.node).elts;
            elts.push_back(std::move($1));
            elts.push_back(std::move($3));
        }
    | assoc_list ',' expr0 ':' expr0
        {
            auto &elts = std::get<AST::List>($1.node).elts;
            elts.push_back(std::move($3));
            elts.push_back(std::move($5));
            $$ = $1;
        }
    ;

aggregation
    : L_ARRAY_OPEN '}' ')'
        { ASSIGN_AST($$, @$, AST::ArrayLiteral{}); }
    | L_ARRAY_OPEN expr_list '}' ')'
        { ASSIGN_AST($$, @$, AST::ArrayLiteral{ std::move(std::get<AST::List>($2.node).elts) }); }
    | L_MAPPING_OPEN assoc_list ']' ')'
        { ASSIGN_AST($$, @$, AST::MappingLiteral{ std::move(std::get<AST::List>($2.node).elts) }); }
    | L_MAPPING_OPEN ']' ')'
        { ASSIGN_AST($$, @$, AST::MappingLiteral{}); }
    ;

defined_id
    : L_IDENTIFIER %prec POSITIONAL_IDENTIFIER
        {
            auto entry = yyget_extra(yyscanner)->current_scope->lookup($1);
            if (!entry) {
                throw parse_error("Undefined identifier: " + std::string($1), @$, yyget_extra(yyscanner));
            }
            $$ = entry;
        }
    | defined_id '.' L_IDENTIFIER
    ;

call_prefix
    : '.'               // member access
    | L_ARROW           // context object access
    | L_COLON_COLON     // context namespace/resource access
    ;

res_acquire
    : defined_id L_COLON_COLON L_IDENTIFIER
        { ASSIGN_AST($$, @$, AST::ResourceAcquire{ std::move($1), yyget_extra(yyscanner)->consts->find_or_add($3) }); }
    | call_prefix L_IDENTIFIER
        { ASSIGN_AST($$, @$, AST::ResourceAcquire{ nullptr, yyget_extra(yyscanner)->consts->find_or_add($2) }); }
    ;

function_call
    : defined_id '(' arg_list ')'
        {
            auto lpcc = yyget_extra(yyscanner);
            if (!$1)
                throw parse_error("Function not found", @$, yyget_extra(yyscanner));
            $1->signature.b.referenced = 1; // mark the function as referenced
            ASSIGN_AST($$, @$, AST::FunctionCall{ std::move($1), std::make_unique<ASTNode>(std::move($3)) });
        }
    | L_NEW '(' res_acquire ')' opt_init_list
        {
            ASSIGN_AST($$, @$, AST::New{
                std::make_unique<ASTNode>(std::move($3)), // clone semantics for the resource
                std::make_unique<ASTNode>(std::move($5))  // initialization list for globals
            });
        }
    | res_acquire '(' arg_list ')'
        {
            ASSIGN_AST($$, @$, AST::ResourceCall{
                std::make_unique<ASTNode>(std::move($1)), // find-or-load semantic for the resource
                std::make_unique<ASTNode>(std::move($3))
            });
        }
    | expr4 L_ARROW L_IDENTIFIER '(' arg_list ')'
        {
            ASSIGN_AST($$, @$, AST::CallOther{
                std::make_unique<ASTNode>(std::move($1)), // dereference the object to call the method on
                yyget_extra(yyscanner)->consts->find_or_add($3),
                std::make_unique<ASTNode>(std::move($5))
            });
        }
    ;

opt_init_list
    : %empty
        { ASSIGN_AST($$, @$, AST::List{}); }
    | '{' arg_list '}'
        { ASSIGN_AST($$, @$, AST::List{ std::move(std::get<AST::List>($2.node).elts) }); }
    ;

kwarg
    : L_IDENTIFIER '=' expr0
        {
            ASSIGN_AST($$, @$, AST::KeywordArgument{ $1, *yyget_extra(yyscanner)->consts });
            std::get<AST::KeywordArgument>($$.node).value = std::make_unique<ASTNode>(std::move($3));
        }
    ;

argument
    : expr0
    | kwarg
    ;

arguments
    : argument
        {
            ASSIGN_AST($$, @$, AST::List{});
            std::get<AST::List>($$.node).elts.push_back(std::move($1));
        }
    | arguments ',' argument
        {
            auto& args = std::get<AST::List>($1.node).elts;
            if (std::holds_alternative<AST::KeywordArgument>(args.back().node)
                && !std::holds_alternative<AST::KeywordArgument>($3.node))
                throw parse_error("Positional argument after keyword argument", @3, yyget_extra(yyscanner));
            args.push_back(std::move($3));
            $$ = $1;
        }
    ;

arg_list
    : %empty
        { ASSIGN_AST($$, @$, AST::List{}); }
    | arguments
    ;

%%

// User subroutines section
void yy::LpcParser::error(const location_type& location, const std::string &msg) {
    throw parse_error(msg, location, yyget_extra(yyscanner));
}
