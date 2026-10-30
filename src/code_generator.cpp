// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "code_generator.hpp"
#include <algorithm>
#include <stdexcept>
#include <variant>

using namespace lpcvm;

void code_generator::grow_bytecode(std::size_t length) {
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

code_generator& code_generator::emit_statement(const ASTNode& stmt) {
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

code_generator& code_generator::emit_expression(const ASTNode& expr) {
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
        [&](const AST::BinaryOp& bin_op) {
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

code_generator& code_generator::emit_block(const AST::Block& block, std::size_t num_formal_params) {
    if (!block.stmts || !std::holds_alternative<AST::List>(block.stmts->node))
        return *this;

    // when entering the block, allocate space for local variables (excluding formal parameters)
    uint32_t num_locals = block.num_locals;
    if (num_formal_params > num_locals)
        throw std::runtime_error("Number of formal parameters exceeds number of local variables");
    num_locals -= num_formal_params; // arguments are pushed by the caller
    if (num_locals > 0)
        emit(F_PUSH_N, num_locals);

    // emit code for each statement in the block
    auto& stmts = std::get<AST::List>(block.stmts->node).elts;
    for(auto& stmt: stmts)
        emit_statement(stmt);

    // when exiting the block, deallocate space for local variables (excluding formal parameters)
    // TODO: back patch for jump instructions that may need to skip over the local variable deallocation.
    if (num_locals > 0)
        emit(F_POP_N, num_locals);
    return *this;
}

void function_builder::build(const AST::FunctionDef& func_def) {
    // AST::FunctionDef contains the function declaration and body.
    // - The function index (assigned when parsing) is retrieved from the function name's
    //   symbol table entry associated with the function declaration.
    // - The number of formal parameters is determined from the function declaration's
    //   parameter list.
    const AST::FunctionDecl& func_decl = std::get<AST::FunctionDecl>(func_def.decl->node);
    auto func_idx = std::get<uint32_t>(*func_decl.entry->value);
    std::size_t num_formal_params = std::get<AST::ParameterList>(func_decl.param_list->node).parameters.size();

    VirtualTable::Binding func_binding;
    func_binding.signature = func_decl.entry->signature;
    func_binding.inheriting = -1; // or some appropriate default value indicating no inheritance
    func_binding.bytecode_offset = codegen_.position(); // entry point of the function's bytecode

    // generate bytecode for the function body
    const AST::Block& body = std::get<AST::Block>(func_def.body->node);
    codegen_.emit_block(body, num_formal_params);
    func_binding.bytecode_length = codegen_.position() - func_binding.bytecode_offset;

    // register function bindings
    vtable_.bind(func_idx, func_binding);
    if (name_idx_ != ConstPool::INVALID_INDEX) {
        // TODO: Handle the case where the function has a name and should be added to the virtual table by name
        vtable_.emplace(name_idx_, func_binding);
    }
}
