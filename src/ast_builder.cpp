// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "ast_builder.hpp"
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"

AstFunctionNode::AstFunctionNode(yy::ast_context& astctx, std::string name, SymbolSignature signature)
    : AstNode(Type::FunctionDecl), name_(std::move(name)), signature_(signature), num_locals_(0) {

}

AstBlockNode::AstBlockNode(yy::ast_context& astctx)
    : AstNode(Type::Block) {
}

AstVarDeclNode::AstVarDeclNode(yy::ast_context& astctx, std::string name, SymbolSignature signature)
    : AstNode(Type::NewLocal), name_(std::move(name)), signature_(signature) {

}

AstAssignNode::AstAssignNode(yy::ast_context& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs)
    : AstNode(Type::Assign) {
    add_child(lhs);
    add_child(rhs);
}

AstUnaryOpNode::AstUnaryOpNode(yy::ast_context& astctx, std::shared_ptr<AstNode> operand)
    : AstNode(Type::UnaryOp) {
    add_child(operand);
}

AstBinaryOpNode::AstBinaryOpNode(yy::ast_context& astctx, std::shared_ptr<AstNode> lhs, std::shared_ptr<AstNode> rhs)
    : AstNode(Type::BinaryOp) {
    add_child(lhs);
    add_child(rhs);
}

AstTernaryOpNode::AstTernaryOpNode(yy::ast_context& astctx, std::shared_ptr<AstNode> condition, std::shared_ptr<AstNode> true_expr, std::shared_ptr<AstNode> false_expr)
    : AstNode(Type::TernaryOp) {
    add_child(condition);
    add_child(true_expr);
    add_child(false_expr);
}

AstConstantNode::AstConstantNode(yy::ast_context& astctx, ConstPool::IndexType index)
    : AstNode(Type::Constant), index_(index) {

}

AstVariableNode::AstVariableNode(yy::ast_context& astctx, std::string name)
    : AstNode(Type::Variable), name_(std::move(name)) {

}

AstFunctionCallNode::AstFunctionCallNode(yy::ast_context& astctx, std::string name)
    : AstNode(Type::FunctionCall), name_(std::move(name)) {

}
