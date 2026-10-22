// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "bytecode_generator.hpp"
#include <variant>

using namespace lpcvm;

BytecodeGenerator& BytecodeGenerator::emit_statement(const ASTNode& stmt) {
    context_ = STATEMENT;
    std::visit(overloaded {
        [&](const AST::Return& ret_stmt) {
            if (ret_stmt.expr) {
                emit_expression(*ret_stmt.expr);
                emit(F_RETURN, 1);
            }
            else {
                emit(F_RETURN, 0);
            }
        },
        [&](auto& expr) {
            emit_expression(stmt);
            emit(F_POP_N, 1); // Pop the result of the expression if it's a statement
        }
    }, stmt.node);
    return *this;
}

BytecodeGenerator& BytecodeGenerator::emit_expression(const ASTNode& expr) {
    context_ = EXPRESSION;
    std::visit(overloaded {
        [&](const AST::NumericConst& constant) {
            std::visit(overloaded {
                [&](const int32_t& int_const)       { emit(F_CONST_I4, constant.index); },
                [&](const uint32_t& uint_const)     { emit(F_CONST_U4, constant.index); },
                [&](const int64_t& int64_const)     { emit(F_CONST_I8, constant.index); },
                [&](const uint64_t& uint64_const)   { emit(F_CONST_U8, constant.index); },
                [&](const float& float_const)       { emit(F_CONST_F4, constant.index); },
                [&](const double& double_const)     { emit(F_CONST_F8, constant.index); },
            }, constant.value);
        },
        [&](const AST::BinaryOp& bin_op) {
            emit_expression(*bin_op.lhs);
            emit_expression(*bin_op.rhs);
            emit(F_BIN_OP, bin_op.arg);
        },
        [&](auto&) {
            // Handle other expression types
        }
    }, expr.node);
    return *this;
}

FunctionBuilder& FunctionBuilder::emit_body(const AST::Block& body) {
    if (body.stmts.empty())
        return *this;
    for(auto& stmt: body.stmts)
        emit_statement(stmt);
    return *this;
}
