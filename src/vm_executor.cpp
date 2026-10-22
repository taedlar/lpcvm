// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <stdexcept>
#include <type_traits>

/**
 * @brief BytecodeView::execute() implementation (a token-threaded virtual machine)
 *
 * VM_LOOP_START and VM_LOOP_END macros are used to define the main execution loop of the VM.
 * VM_HANDLER macro is used to define the handler for each opcode.
 * VM_DISPATCH macro is used to jump to the next instruction handler.
 *
 * @param stack The evaluation stack used by the VM.
 * @param num_args The number of arguments expected on the stack.
 * @return The number of return values on the stack. Arguments are popped before returning
 * as like __stdcall convention. (The calee may reuse the stack space for locals)
 */
int BytecodeView::execute(EvaluationStack& stack, int num_args) {
    std::size_t initial_sp = stack.size();
    if (initial_sp < num_args)
        throw std::runtime_error("Not enough arguments on the stack");
#if defined(__GNUC__) || defined(__clang__)
    #define VM_HAS_COMPUTED_GOTO 1
#else
    #define VM_HAS_COMPUTED_GOTO 0
#endif

#if VM_HAS_COMPUTED_GOTO
    // Computed goto is supported
    #define AS_LABEL(op) &&do_##op,
    #define VM_LOOP_START() \
        static const void* dispatch_table[256] = { \
            ALL_OPCODES(AS_LABEL) \
        }; \
        for (int i = Opcode::INVALID_VALUE; i < 256; ++i) { dispatch_table[i] = &&invalid_opcode; } \
        VM_DISPATCH();
    #define VM_LOOP_END()
    #define VM_HANDLER(op) do_##op:
    #define VM_DISPATCH() \
        extended_arg = 0; \
        if (pc < end_) goto *dispatch_table[*pc++]; else goto end_of_bytecode;
    #define VM_DISPATCH_EXTENDED() \
        if (pc < end_) goto *dispatch_table[*pc++]; else goto end_of_bytecode;
#else
    // Computed goto is not supported, using switch-case instead
    #define VM_LOOP_START() \
        while (pc < end_) { \
            switch (*pc++) {
    #define VM_LOOP_END() \
            default: \
                goto invalid_opcode; \
            } \
        }
    #define VM_HANDLER(op) case op:
    #define VM_DISPATCH() \
        extended_arg = 0; \
        break
    #define VM_DISPATCH_EXTENDED() \
        break
#endif

    const uint8_t *pc = start_;
    uint32_t extended_arg = 0; // holds the extended argument for the next opcode (CPython style)
    uint32_t oparg; // holds the extendable argument for the current opcode
    std::size_t sp; // stack pointer, this must be fetched in each instruction handler
    SValue result;
    VM_LOOP_START()

    VM_HANDLER(F_RETURN) {
        oparg = (extended_arg << 8) | *pc++; // number of return values on the stack
        std::size_t pre_exec_sp = initial_sp - num_args;
        if (stack.size() - oparg < pre_exec_sp)
            throw std::runtime_error("Stack underflow on return");        
        stack.pop_i(pre_exec_sp, stack.size() - oparg - pre_exec_sp); // pop arguments and locals
        return oparg;
    }

    VM_HANDLER(F_EXTENDED_ARG) {
        extended_arg = (extended_arg << 8) | *pc++; // extended argument for the next opcode (CPython style)
        VM_DISPATCH_EXTENDED();
    }

    VM_HANDLER(F_PUSH_N) {
        oparg = (extended_arg << 8) | *pc++; // number of local variables to push onto the VM stack
        stack.push_n(oparg);
        VM_DISPATCH();
    }

    VM_HANDLER(F_POP_N) {
        oparg = (extended_arg << 8) | *pc++; // number of local variables to pop from the VM stack
        stack.pop_n(oparg);
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_I4) {
        oparg = (extended_arg << 8) | *pc++;
        int32_t value = const_pool_->fetch<int32_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_I8) {
        oparg = (extended_arg << 8) | *pc++;
        int64_t value = const_pool_->fetch<int64_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_U4) {
        oparg = (extended_arg << 8) | *pc++;
        uint32_t value = const_pool_->fetch<uint32_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_U8) {
        oparg = (extended_arg << 8) | *pc++;
        uint64_t value = const_pool_->fetch<uint64_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_F4) {
        oparg = (extended_arg << 8) | *pc++;
        float value = const_pool_->fetch<float>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_F8) {
        oparg = (extended_arg << 8) | *pc++;
        double value = const_pool_->fetch<double>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_STR) {
        oparg = (extended_arg << 8) | *pc++;
        auto value = const_pool_->fetch_string(oparg); // TODO: push a string value onto the VM stack
        VM_DISPATCH();
    }

    VM_HANDLER(F_BIN_OP) {
        oparg = (extended_arg << 8) | *pc++; // fetch the binary operation code
        sp = stack.size();
        auto& rhs = stack[sp - 1];
        auto& lhs = stack[sp - 2];
        result = std::visit([&](const auto& lhs_value, const auto& rhs_value) -> SValue {
            using LhsType = std::decay_t<decltype(lhs_value)>;
            using RhsType = std::decay_t<decltype(rhs_value)>;
            if constexpr (std::is_same_v<LhsType, RhsType> &&
                (std::is_same_v<LhsType, int32_t> || std::is_same_v<LhsType, uint32_t>)) {
                switch (oparg) {
                    case oparg::ADD:
                        return SValue(lhs_value + rhs_value);
                    case oparg::SUB:
                        return SValue(lhs_value - rhs_value);
                    case oparg::MUL:
                        return SValue(lhs_value * rhs_value);
                    case oparg::DIV:
                        return SValue(lhs_value / rhs_value);
                    default:
                        throw std::runtime_error("Invalid binary operation");
                }
            }
            else {
                throw std::runtime_error("Unsupported binary operand types");
            }
        }, lhs, rhs);
        stack.pop_n(2);
        stack.push(result);
        VM_DISPATCH();
    }

    VM_LOOP_END()

invalid_opcode:
    throw std::runtime_error("Invalid opcode");

end_of_bytecode:
    // stops execution if end of bytecode is reached without encountering op_return
    stack.pop_n(stack.size() - (initial_sp - num_args));
    return 0;

#undef VM_LOOP_START
#undef VM_LOOP_END
#undef VM_HANDLER
#undef VM_DISPATCH
#undef VM_HAS_COMPUTED_GOTO
} // BytecodeView::execute()
