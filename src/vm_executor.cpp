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
 * @param control The return stack used by the VM.
 * @param stack The data stack used by the VM.
 * @param num_args The number of arguments on the stack.
 * @return The number of return values on the stack. Arguments are popped before returning
 * as like __stdcall convention. (The calee may reuse the stack space for locals)
 */
int BytecodeView::execute (ReturnStack& control, DataStack& stack, std::size_t num_args) {
    if (!start_ || start_ >= end_) {
        stack.pop_n(num_args);
        return 0; // No bytecode to execute
    }

    auto entry_frame = control.size();
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
        if (pc < end_) { goto *dispatch_table[*pc++]; } else { pc = nullptr; goto do_F_RETURN; }
    #define VM_DISPATCH_EXTENDED() \
        if (pc < end_) { goto *dispatch_table[*pc++]; } else { pc = nullptr; goto do_F_RETURN; }
#else
    // Computed goto is not supported, using switch-case instead
    #define VM_LOOP_START() \
        static uint8_t force_return[] = { F_RETURN, 0 }; \
        do { \
            switch (*pc++) {
    #define VM_LOOP_END() \
            default: \
                goto invalid_opcode; \
            } \
            if (pc >= end_) \
                pc = &force_return[0]; \
        } while (true);
    #define VM_HANDLER(op) case op:
    #define VM_DISPATCH() \
        extended_arg = 0; \
        break;
    #define VM_DISPATCH_EXTENDED() \
        break;
#endif

    if (stack.size() < num_args)
        throw std::runtime_error("Not enough arguments on the stack");
    std::size_t fp = stack.size() - num_args; // frame pointer, points to the base of the current function's stack frame (first argument)
    const uint8_t *pc = start_; // program counter, points to the current instruction in the bytecode
    uint32_t extended_arg = 0; // holds the extended argument for the next opcode (CPython style)
    uint32_t oparg; // holds the extendable argument for the current opcode
    SValue result;
    VM_LOOP_START()

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
        int32_t value = prog_->const_pool()->fetch<int32_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_I8) {
        oparg = (extended_arg << 8) | *pc++;
        int64_t value = prog_->const_pool()->fetch<int64_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_U4) {
        oparg = (extended_arg << 8) | *pc++;
        uint32_t value = prog_->const_pool()->fetch<uint32_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_U8) {
        oparg = (extended_arg << 8) | *pc++;
        uint64_t value = prog_->const_pool()->fetch<uint64_t>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_F4) {
        oparg = (extended_arg << 8) | *pc++;
        float value = prog_->const_pool()->fetch<float>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_F8) {
        oparg = (extended_arg << 8) | *pc++;
        double value = prog_->const_pool()->fetch<double>(oparg);
        stack.push(SValue(value));
        VM_DISPATCH();
    }

    VM_HANDLER(F_CONST_STR) {
        oparg = (extended_arg << 8) | *pc++;
        auto value = prog_->const_pool()->fetch_string(oparg); // TODO: push a string value onto the VM stack
        VM_DISPATCH();
    }

    VM_HANDLER(F_LOAD_GLOBAL) {
        oparg = (extended_arg << 8) | *pc++;
        if (!this_obj_)
            throw std::runtime_error("Global variable access requires an object state");
        stack.push((*this_obj_)[oparg]);
        VM_DISPATCH();
    }

    VM_HANDLER(F_STORE_GLOBAL) {
        oparg = (extended_arg << 8) | *pc++;
        if (!this_obj_)
            throw std::runtime_error("Global variable access requires an object state");
        (*this_obj_)[oparg] = stack.top();
        VM_DISPATCH();
    }

    VM_HANDLER(F_LOAD_LOCAL) {
        oparg = (extended_arg << 8) | *pc++;
        result = stack[fp + oparg];
        stack.push(result);
        VM_DISPATCH();
    }

    VM_HANDLER(F_STORE_LOCAL) {
        oparg = (extended_arg << 8) | *pc++;
        stack[fp + oparg] = stack.top();
        VM_DISPATCH();
    }

    VM_HANDLER(F_BIN_OP) {
        oparg = (extended_arg << 8) | *pc++; // fetch the binary operation code
        auto sp = stack.size();
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

    VM_HANDLER(F_PUSH_FUNC) {
        oparg = (extended_arg << 8) | *pc++;
        stack.push(oparg); // push the function index onto the stack
        // control.push(nullptr, end_, fp);
        // fp = stack.size() - oparg;
        VM_DISPATCH();
    }

    VM_HANDLER(F_CALL) {
        oparg = (extended_arg << 8) | *pc++; // number of items on the stack for this call (func_index + arguments)
        if (oparg < 1)
            throw std::runtime_error("Invalid call with no function index on the stack");
        auto func_index = std::get<uint32_t>(stack[stack.size() - oparg]);
        auto binding = prog_->get_method(func_index);
        control.push(pc, fp, start_, end_); // save the current execution context on the return stack
        fp = stack.size() - oparg + 1; // set the frame pointer for the new function call (fp -> first argument)
        pc = start_ = prog_->bytecode() + binding.bytecode_offset;
        end_ = start_ + binding.bytecode_length;
        VM_DISPATCH();
    }

    VM_HANDLER(F_RETURN) {
        oparg = pc ? (extended_arg << 8) | *pc++ : 0; // number of return values on the stack
        if (oparg > stack.size() || stack.size() - oparg < fp)
            throw std::runtime_error("Stack underflow on return");
        if (control.size() > entry_frame)
            fp--; // if we are returning from F_CALL, adjust the frame pointer accordingly
        stack.pop_i(fp, stack.size() - oparg - fp); // clean up the stack for the current frame
        if (control.size() == entry_frame) {
            // we are done with the VM execution, now return to C++ caller.
            return oparg;
        }
        pc = control.top().pc;
        fp = control.top().fp;
        start_ = control.top().start;
        end_ = control.top().end;
        control.pop();
        VM_DISPATCH();
    }

    VM_LOOP_END()
    return 0;

invalid_opcode:
    throw std::runtime_error("Invalid opcode");

#undef VM_LOOP_START
#undef VM_LOOP_END
#undef VM_HANDLER
#undef VM_DISPATCH
#undef VM_HAS_COMPUTED_GOTO
} // BytecodeView::execute()
