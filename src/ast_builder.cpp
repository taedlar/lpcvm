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

AstAssignNode::AstAssignNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs)
    : AstNode(astctx.lpcc->current_lineno, Type::Assign) {
    add_child(lhs);
    add_child(rhs);
}

AstUnaryOpNode::AstUnaryOpNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> operand)
    : AstNode(astctx.lpcc->current_lineno, Type::UnaryOp) {
    add_child(operand);
}

AstBinaryOpNode::AstBinaryOpNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs)
    : AstNode(astctx.lpcc->current_lineno, Type::BinaryOp) {
    add_child(lhs);
    add_child(rhs);
}

AstTernaryOpNode::AstTernaryOpNode(AstBuilderContext& astctx, std::shared_ptr<AstNode> condition, std::shared_ptr<AstNode> true_expr, std::shared_ptr<AstNode> false_expr)
    : AstNode(astctx.lpcc->current_lineno, Type::TernaryOp) {
    add_child(condition);
    add_child(true_expr);
    add_child(false_expr);
}

AstConstantNode::AstConstantNode(AstBuilderContext& astctx, ConstPool::IndexType index)
    : AstNode(astctx.lpcc->current_lineno, Type::Constant), index_(index) {

}

AstVariableNode::AstVariableNode(AstBuilderContext& astctx, std::string name)
    : AstNode(astctx.lpcc->current_lineno, Type::Variable), name_(std::move(name)) {

}

AstFunctionCallNode::AstFunctionCallNode(AstBuilderContext& astctx, std::string name)
    : AstNode(astctx.lpcc->current_lineno, Type::FunctionCall), name_(std::move(name)) {

}
