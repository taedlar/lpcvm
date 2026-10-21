// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <stdexcept>

/**
 * BytecodeView::execute() implementation (a token-threaded virtual machine)
 *
 * VM_LOOP_START and VM_LOOP_END macros are used to define the main execution loop of the VM.
 * VM_HANDLER macro is used to define the handler for each opcode.
 * VM_DISPATCH macro is used to jump to the next instruction handler.
 */
void BytecodeView::execute(EvaluationStack& stack) {
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
    VM_LOOP_START()

    VM_HANDLER(F_RETURN) {
        uint32_t num_ret = (extended_arg << 8) | *pc++; // number of return values on the stack
        return;
    }

    VM_HANDLER(F_EXTENDED_ARG) {
        extended_arg = (extended_arg << 8) | *pc++; // extended argument for the next opcode (CPython style)
        VM_DISPATCH_EXTENDED();
    }

    VM_HANDLER(F_PUSH_N) {
        uint32_t num_locals = (extended_arg << 8) | *pc++; // number of local variables to push onto the VM stack
        for (uint32_t i = 0; i < num_locals; ++i) {
            stack.push(SValue());
        }
        VM_DISPATCH();
    }

    VM_HANDLER(F_POP_N) {
        uint32_t num_locals = (extended_arg << 8) | *pc++; // number of local variables to pop from the VM stack
        for (uint32_t i = 0; i < num_locals; ++i) {
            stack.pop(); // dtor of svalue variant will be called here, freeing any resources held by the value
        }
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_I4) {
        uint32_t index = (extended_arg << 8) | *pc++;
        int32_t value = const_pool_->fetch<int32_t>(index);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_I8) {
        uint32_t index = (extended_arg << 8) | *pc++;
        int64_t value = const_pool_->fetch<int64_t>(index);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_U4) {
        uint32_t index = (extended_arg << 8) | *pc++;
        uint32_t value = const_pool_->fetch<uint32_t>(index);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_U8) {
        uint32_t index = (extended_arg << 8) | *pc++;
        uint64_t value = const_pool_->fetch<uint64_t>(index);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_F4) {
        uint32_t index = (extended_arg << 8) | *pc++;
        float value = const_pool_->fetch<float>(index);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_F8) {
        uint32_t index = (extended_arg << 8) | *pc++;
        double value = const_pool_->fetch<double>(index);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_STR) {
        uint32_t index = (extended_arg << 8) | *pc++;
        auto value = const_pool_->fetch_string(index); // TODO: push a string value onto the VM stack
        VM_DISPATCH();
    }

    VM_HANDLER(F_BIN_OP) {
        uint32_t oparg = (extended_arg << 8) | *pc++; // fetch the binary operation code
        auto rhs = stack.pop();
        auto lhs = stack.pop();
        SValue result;
        switch (oparg) {
            case BinaryOparg::ADD:
                result = std::get<int32_t>(lhs) + std::get<int32_t>(rhs);
                break;
            case BinaryOparg::SUB:
                result = std::get<int32_t>(lhs) - std::get<int32_t>(rhs);
                break;
            case BinaryOparg::MUL:
                result = std::get<int32_t>(lhs) * std::get<int32_t>(rhs);
                break;
            case BinaryOparg::DIV:
                result = std::get<int32_t>(lhs) / std::get<int32_t>(rhs);
                break;
            default:
                throw std::runtime_error("Invalid binary operation");
        }
        stack.push(result);
        VM_DISPATCH();
    }

    VM_LOOP_END()

invalid_opcode:
    throw std::runtime_error("Invalid opcode");

end_of_bytecode:
    // stops execution if end of bytecode is reached without encountering op_return
    return;

#undef VM_LOOP_START
#undef VM_LOOP_END
#undef VM_HANDLER
#undef VM_DISPATCH
#undef VM_HAS_COMPUTED_GOTO
} // BytecodeView::execute()
