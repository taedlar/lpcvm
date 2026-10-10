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
    #include "symbol_table.hpp"

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

    struct ast_builder_context_s {
        YY_EXTRA_TYPE lpcc;
    };

    /// @brief Helper function to create a new AST node.
    /// @tparam T The type of the AST node to create.
    /// @tparam Args The types of the arguments to forward to the AST node constructor.
    /// @param yyscanner The reentrant lexer handle.
    /// @param args The arguments to forward to the AST node constructor.
    /// @return A shared pointer to the newly created AST node.
    template<typename T, typename... Args>
    static std::shared_ptr<T> new_node(yyscan_t yyscanner, Args&&... args) {
        ast_builder_context_s astctx;
        astctx.lpcc = yyget_extra(yyscanner);
        astctx.lpcc->current_lineno = yyget_lineno(yyscanner);
        astctx.lpcc->current_column = yyget_column(yyscanner);
        return std::make_shared<T>(astctx, std::forward<Args>(args)...);
    }
}

%code top {
    /*
    * SPDX-License-Identifier: MIT
    * SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
    */
    #include "ast_builder.hpp"
    #include "const_pool.hpp"
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

%type <std::shared_ptr<AstNode>> program def
%type <std::shared_ptr<AstNode>> function_head
%type <std::shared_ptr<AstNode>> local_decl
%type <std::shared_ptr<AstNode>> var_list
%type <std::shared_ptr<AstNode>> comma_expr lvalue expr0 expr4
%type <std::shared_ptr<AstNode>> function_call
%type <std::vector<std::shared_ptr<AstNode>>> arg_list

%type <SymbolSignature> storage_or_type typed_storage
%type <SymbolSignature> type_modifier_list opt_star

%type <std::shared_ptr<SymbolEntry>> new_var

%type <std::string> str_literal str_const

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
        }
      '(' opt_parameter_list ')'
        {
            /* handle function parameters */
        }
      block_or_semicolon
        {
            /* handle forward declaration or definition for the function */
            $$ = $1;
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
            $1.b.data_type = LpcDataType::T_INT; // implicit function return type
            $$ = new_node<AstFunctionNode>(yyscanner, $2, $1);
        }
    | storage_or_type opt_star L_IDENTIFIER
        {
            $1.value |= $2.value;
            $$ = new_node<AstFunctionNode>(yyscanner, $3, $1);
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
        { $$ = new_node<AstVarDeclNode>(yyscanner, $1->get_name(), $1->get_signature()); }
    | var_list ',' new_var
        { ($$ = $1)->add_child(new_node<AstVarDeclNode>(yyscanner, $3->get_name(), $3->get_signature())); }
    ;

new_var
    : opt_star L_IDENTIFIER
        { $$ = std::make_shared<SymbolEntry>($2, $1); }
    | opt_star L_IDENTIFIER L_ASSIGN expr0
        { $$ = std::make_shared<SymbolEntry>($2, $1); }
    ;

inheritance
    : L_INHERIT str_const ';'
    | type_modifier_list L_INHERIT str_const ';'
    ;

str_const
    : L_STRING_LITERAL
        { $$ = $1; }
    | '(' str_const ')'
        { $$ = $2; }
    | str_const L_STRING_LITERAL
        { $$ += $2; /* string literals concatenation */ }
    ;

str_literal
    : L_STRING_LITERAL
        { $$ = $1; }
    | str_literal L_STRING_LITERAL
        { $$ = $1 + $2; /* string literals concatenation */ }
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
        { $$ = $1; }
    | comma_expr ',' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    ;

expr0
    : lvalue L_ASSIGN expr0
        { $$ = new_node<AstAssignNode>(yyscanner, $1, $3); }
    | expr0 '?' expr0 ':' expr0 %prec '?'
        { $$ = new_node<AstTernaryOpNode>(yyscanner, $1, $3, $5); }
    | expr0 L_LOR expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 L_LAND expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '|' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '^' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '&' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 L_LSH expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 L_RSH expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '+' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '-' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '*' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '/' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 '%' expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | expr0 L_ORDER expr0
        { $$ = new_node<AstBinaryOpNode>(yyscanner, $1, $3); }
    | L_INC lvalue %prec L_NOT
        { $$ = new_node<AstAssignNode>(yyscanner, nullptr, $2); }
    | L_DEC lvalue %prec L_NOT
        { $$ = new_node<AstAssignNode>(yyscanner, nullptr, $2); }
    | lvalue L_INC
        { $$ = new_node<AstAssignNode>(yyscanner, $1, nullptr); }
    | lvalue L_DEC
        { $$ = new_node<AstAssignNode>(yyscanner, $1, nullptr); }
    | L_NOT expr0
        { $$ = new_node<AstUnaryOpNode>(yyscanner, $2); }
    | '~' expr0
        { $$ = new_node<AstUnaryOpNode>(yyscanner, $2); }
    | '-' expr0 %prec L_NOT
        { $$ = new_node<AstUnaryOpNode>(yyscanner, $2); }
    | expr4
    | L_INTEGER
        { auto idx = yyget_extra(yyscanner)->consts->find_or_add($1); $$ = new_node<AstConstantNode>(yyscanner, idx); }
    | L_REAL_NUMBER
        { auto idx = yyget_extra(yyscanner)->consts->find_or_add($1); $$ = new_node<AstConstantNode>(yyscanner, idx); }    
    | str_literal
        { auto idx = yyget_extra(yyscanner)->consts->find_or_add($1); $$ = new_node<AstConstantNode>(yyscanner, idx); }
    ;

lvalue
    : expr4
        { $$ = $1; }
    ;

expr4
    : '(' comma_expr ')'
        { $$ = $2; }
    | L_IDENTIFIER
        { $$ = new_node<AstVariableNode>(yyscanner, $1); }
    | function_call
        { $$ = $1; }    
    | expr4 '[' comma_expr ']'
    ;

local_decl
    : storage_or_type var_list ';'
        { $$ = $2; }
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
            $$ = new_node<AstFunctionCallNode>(yyscanner, $1);
            for (auto& arg : $3) {
                $$->add_child(arg);
            }
        }
    ;

arg_list
    : %empty
        { $$ = std::vector<std::shared_ptr<AstNode>>(); }
    | expr0
        { $$ = std::vector<std::shared_ptr<AstNode>>({$1}); }
    | arg_list ',' expr0
        { ($$ = $1).push_back($3); }
    ;

%%

// User subroutines section
void yy::LpcParser::error(const std::string &msg) {
    // YY_EXTRA_TYPE lpcc = yyget_extra(yyscanner);
    throw std::runtime_error("Line " + std::to_string(yyget_lineno(yyscanner)) + ": " + msg);
}

AstProgramNode::AstProgramNode()
    : AstNode(0, AstNode::Type::Program) {
}
