// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "ast_node.hpp"
#include "const_pool.hpp"
#include "symbol_table.hpp"
#include "vm_runtime_class.hpp"

/// @brief Context information passed to AST builder nodes.
/// @details This structure contains information such as the current line number,
/// the current column, and a pointer to the compiler context (LpcCompiler::Context*).
/// The AST builder nodes use this context to access essential compile-time information
/// such as symbol tables, constant pools, and the current program state.
namespace yy {
    struct ast_context; // see lpc_parser.yy for the full definition of ast_context
}

// =======================================================================
// AST builder node classes for the LPCVM compiler.

/// @brief AST node representing a function definition.
class AstFunctionDefNode : public AstNode {
public:
    AstFunctionDefNode (yy::ast_context& astctx, std::string name, SymbolSignature signature);
    void generate_code(std::vector<uint8_t>& code) override {}

    const std::string& get_name() const { return name_; }
    const SymbolSignature& get_signature() const { return signature_; }

private:
    std::string name_;
    SymbolSignature signature_;
};

/// @brief AST node representing a block of statements.
class AstBlockNode : public AstNode {
public:
    AstBlockNode(yy::ast_context& astctx);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a variable declaration.
class AstVarDeclNode : public AstNode {
public:
    AstVarDeclNode(yy::ast_context& astctx, std::string name, SymbolSignature signature);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
    SymbolSignature signature_;
};

/// @brief AST node representing an assignment operation.
class AstAssignNode : public AstNode {
public:
    AstAssignNode(yy::ast_context& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a unary operation.
class AstUnaryOpNode : public AstNode {
public:
    AstUnaryOpNode(yy::ast_context& astctx, std::shared_ptr<AstNode> operand);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a binary operation.
class AstBinaryOpNode : public AstNode {
public:
    AstBinaryOpNode(yy::ast_context& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a ternary operation.
class AstTernaryOpNode : public AstNode {
public:
    AstTernaryOpNode(yy::ast_context& astctx, std::shared_ptr<AstNode> condition, std::shared_ptr<AstNode> true_expr, std::shared_ptr<AstNode> false_expr);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a constant value.
class AstConstantNode : public AstNode {
public:
    AstConstantNode(yy::ast_context& astctx, ConstPool::IndexType index);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    ConstPool::IndexType index_;
};

/// @brief AST node representing a variable.
class AstVariableNode : public AstNode {
public:
    AstVariableNode(yy::ast_context& astctx, std::string name);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
};

/// @brief AST node representing a function call.
class AstFunctionCallNode : public AstNode {
public:
    AstFunctionCallNode(yy::ast_context& astctx, std::string name);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
};
