// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
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
