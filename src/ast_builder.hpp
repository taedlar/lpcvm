// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "const_pool.hpp"
#include "symbol_table.hpp"
#include "ast_node.hpp"

typedef struct ast_builder_context_s AstBuilderContext;

/// @brief AST node representing a function declaration or definition.
class AstFunctionNode : public AstNode {
public:
    AstFunctionNode (AstBuilderContext& astctx, std::string name, SymbolSignature signature);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
    SymbolSignature signature_;
};

/// @brief AST node representing a variable declaration.
class AstVarDeclNode : public AstNode {
public:
    AstVarDeclNode(AstBuilderContext& astctx, std::string name, SymbolSignature signature);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
    SymbolSignature signature_;
};

/// @brief AST node representing an assignment operation.
class AstAssignNode : public AstNode {
public:
    AstAssignNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a unary operation.
class AstUnaryOpNode : public AstNode {
public:
    AstUnaryOpNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> operand);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a binary operation.
class AstBinaryOpNode : public AstNode {
public:
    AstBinaryOpNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a ternary operation.
class AstTernaryOpNode : public AstNode {
public:
    AstTernaryOpNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> condition, std::shared_ptr<AstNode> true_expr, std::shared_ptr<AstNode> false_expr);
    void generate_code(std::vector<uint8_t>& code) override {}
};

/// @brief AST node representing a constant value.
class AstConstantNode : public AstNode {
public:
    AstConstantNode(AstBuilderContext& astctx, ConstPool::IndexType index);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    ConstPool::IndexType index_;
};

/// @brief AST node representing a variable.
class AstVariableNode : public AstNode {
public:
    AstVariableNode(AstBuilderContext& astctx, std::string name);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
};

/// @brief AST node representing a function call.
class AstFunctionCallNode : public AstNode {
public:
    AstFunctionCallNode(AstBuilderContext& astctx, std::string name);
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
};
