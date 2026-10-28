// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "bytecode_generator.hpp"
#include <stdexcept>
#include <variant>

using namespace lpcvm;

BytecodeGenerator& BytecodeGenerator::emit_statement(const ASTNode& stmt) {
    context_ = STATEMENT;
    std::visit(overloaded {
        [&](const AST::Block& block) {
            emit_block(block);
        },
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
        [&](const AST::FunctionCall& func_call) {
            auto func_idx = std::get<uint32_t>(*func_call.entry->value);
            emit(F_PUSH_FUNC, func_idx);
            const auto& args = std::get<AST::List>(func_call.arguments->node);
            for(auto& arg: args.elts)
                emit_expression(arg);
            // stack: [func_index, arg1, arg2, ...]
            emit(F_CALL, args.elts.size() + 1);
        },
        [&](auto&) {
            // Handle other expression types
        }
    }, expr.node);
    return *this;
}

BytecodeGenerator& BytecodeGenerator::emit_block(const AST::Block& block, std::size_t num_formal_params) {
    if (!block.stmts || !std::holds_alternative<AST::List>(block.stmts->node))
        return *this;
    uint32_t num_locals = block.num_locals;
    if (num_formal_params > num_locals)
        throw std::runtime_error("Number of formal parameters exceeds number of local variables");
    num_locals -= num_formal_params; // arguments are pushed by the caller
    if (num_locals > 0)
        emit(F_PUSH_N, num_locals);
    auto& stmts = std::get<AST::List>(block.stmts->node).elts;
    for(auto& stmt: stmts)
        emit_statement(stmt);
    if (num_locals > 0)
        emit(F_POP_N, num_locals);
    return *this;
}

FunctionBuilder& FunctionBuilder::emit_body(const AST::Block& body, std::size_t num_formal_params) {
    emit_block(body, num_formal_params);
    return *this;
}
