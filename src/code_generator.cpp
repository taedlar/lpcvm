// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "code_generator.hpp"
#include <algorithm>
#include <stdexcept>
#include <variant>

using namespace lpcvm;

void Bytecode::grow_bytecode(std::size_t length) {
    constexpr std::size_t allocation_granularity = 64;
    const auto max_capacity = bytecode_.max_size();
    if (length > max_capacity - bytecode_.size())
        throw std::length_error("Bytecode exceeds maximum size");
    const auto capacity = bytecode_.capacity();
    const auto growth = std::min(max_capacity - capacity,
        std::max(capacity, allocation_granularity));
    auto new_capacity = std::max(bytecode_.size() + length, capacity + growth);
    const auto padding = (allocation_granularity -
        new_capacity % allocation_granularity) % allocation_granularity;
    if (padding <= max_capacity - new_capacity)
        new_capacity += padding;
    bytecode_.reserve(new_capacity);
}

void Bytecode::emit_variable_load(const AST::Variable& var) {
    if (!var.entry || !var.entry->value || !std::holds_alternative<uint32_t>(*var.entry->value))
        throw std::runtime_error("Variable has no storage index");
    emit(var.entry->signature.b.is_local ? F_LOAD_LOCAL : F_LOAD_GLOBAL, std::get<uint32_t>(*var.entry->value));
}

void Bytecode::emit_variable_store(const AST::Variable& var) {
    if (!var.entry || !var.entry->value || !std::holds_alternative<uint32_t>(*var.entry->value))
        throw std::runtime_error("Variable has no storage index");
    emit(var.entry->signature.b.is_local ? F_STORE_LOCAL : F_STORE_GLOBAL, std::get<uint32_t>(*var.entry->value));
}

Bytecode& Bytecode::emit_statement(const ASTNode& stmt) {
    context_ = STATEMENT;
    std::visit(overloaded {
        [&](const AST::Block& block) {
            emit_block(block);
        },
        [&](const AST::Cond& cond) {
            emit_expression(*cond.condition);
            const auto to_else = emit_jump(F_JUMP_IF_FALSE);
            emit_statement(*cond.body);
            if (cond.else_body && cond.else_body->node.index() != 0) {
                const auto to_end = emit_jump(F_JUMP);
                patch_jump(to_else);
                emit_statement(*cond.else_body);
                patch_jump(to_end);
            }
            else {
                patch_jump(to_else);
            }
        },
        [&](const AST::PreTestedLoop& loop) {
            const auto top = position();
            std::size_t exit_jump = 0;
            const bool has_cond = loop.condition && loop.condition->node.index() != 0;
            if (has_cond) {
                emit_expression(*loop.condition);
                exit_jump = emit_jump(F_JUMP_IF_FALSE);
            }
            loops_.push_back({local_depth_, {}, {}});
            emit_statement(*loop.body);
            auto ctx = std::move(loops_.back());
            loops_.pop_back();
            for (auto pos: ctx.continues)
                patch_jump(pos);
            if (loop.step && loop.step->node.index() != 0)
                emit_statement(*loop.step);
            emit_jump_to(F_JUMP, top);
            if (has_cond)
                patch_jump(exit_jump);
            for (auto pos: ctx.breaks)
                patch_jump(pos);
        },
        [&](const AST::PostTestedLoop& loop) {
            const auto top = position();
            loops_.push_back({local_depth_, {}, {}});
            emit_statement(*loop.body);
            auto ctx = std::move(loops_.back());
            loops_.pop_back();
            for (auto pos: ctx.continues)
                patch_jump(pos);
            emit_expression(*loop.condition);
            const auto exit_jump = emit_jump(F_JUMP_IF_FALSE);
            emit_jump_to(F_JUMP, top);
            patch_jump(exit_jump);
            for (auto pos: ctx.breaks)
                patch_jump(pos);
        },
        [&](const AST::Break&) {
            if (loops_.empty())
                throw std::runtime_error("break statement outside of loop");
            if (local_depth_ > loops_.back().local_depth)
                emit(F_POP_N, local_depth_ - loops_.back().local_depth);
            loops_.back().breaks.push_back(emit_jump(F_JUMP));
        },
        [&](const AST::Continue&) {
            if (loops_.empty())
                throw std::runtime_error("continue statement outside of loop");
            if (local_depth_ > loops_.back().local_depth)
                emit(F_POP_N, local_depth_ - loops_.back().local_depth);
            loops_.back().continues.push_back(emit_jump(F_JUMP));
        },
        [&](const std::monostate&) {
            // empty statement
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

Bytecode& Bytecode::emit_expression(const ASTNode& expr) {
    context_ = EXPRESSION;
    set_source_location(expr);
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
        [&](const AST::Variable& var) {
            emit_variable_load(var);
        },
        [&](const AST::Assign& assign) {
            // unary forms (e.g. ++/--) have a null operand and are not supported yet
            if (!assign.lhs || !assign.rhs)
                return;
            const auto* var = std::get_if<AST::Variable>(&assign.lhs->node);
            if (!var)
                throw std::runtime_error("Unsupported assignment target");
            emit_expression(*assign.rhs);
            set_source_location(expr);
            emit_variable_store(*var); // the assigned value remains on the stack as the expression result
        },
        [&](const AST::BinaryOp& bin_op) {
            if (bin_op.arg == oparg::ASSIGN) { // variable declaration with initializer
                const auto* var = std::get_if<AST::Variable>(&bin_op.lhs->node);
                if (!var)
                    throw std::runtime_error("Unsupported assignment target");
                emit_expression(*bin_op.rhs);
                set_source_location(expr);
                emit_variable_store(*var);
                return;
            }
            emit_expression(*bin_op.lhs);
            emit_expression(*bin_op.rhs);
            set_source_location(expr);
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

Bytecode& Bytecode::emit_block(const AST::Block& block, std::size_t num_formal_params) {
    if (!block.stmts || !std::holds_alternative<AST::List>(block.stmts->node))
        return *this;

    // when entering the block, allocate space for local variables (excluding formal parameters)
    uint32_t num_locals = block.num_locals;
    if (num_formal_params > num_locals)
        throw std::runtime_error("Number of formal parameters exceeds number of local variables");
    num_locals -= num_formal_params; // arguments are pushed by the caller
    if (num_locals > 0)
        emit(F_PUSH_N, num_locals);
    local_depth_ += num_locals;

    // emit code for each statement in the block
    auto& stmts = std::get<AST::List>(block.stmts->node).elts;
    for(auto& stmt: stmts)
        emit_statement(stmt);

    // when exiting the block, deallocate space for local variables (excluding formal parameters)
    // TODO: back patch for jump instructions that may need to skip over the local variable deallocation.
    if (num_locals > 0) {
        emit(F_POP_N, num_locals);
        local_depth_ -= num_locals;
    }
    return *this;
}
