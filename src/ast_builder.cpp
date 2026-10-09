// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "ast_builder.hpp"
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"

AstFunctionNode::AstFunctionNode(AstBuilderContext& astctx, std::string name, SymbolSignature signature)
    : AstNode(astctx.lpcc->current_lineno, Type::Function), name_(std::move(name)), signature_(signature) {

}

AstVarDeclNode::AstVarDeclNode(AstBuilderContext& astctx, std::string name, SymbolSignature signature)
    : AstNode(astctx.lpcc->current_lineno, Type::VariableDeclaration), name_(std::move(name)), signature_(signature) {

}
