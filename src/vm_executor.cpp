// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "object_class.hpp"
#include "object_state.hpp"
#include "vm_exception.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <stdexcept>
#include <type_traits>
#include <variant>

/// @brief The common type of two arithmetic operands (usual arithmetic conversions).
/// Floating point wins; mixed-sign integers promote to a signed type that can hold both
/// (int64 for 32-bit unsigned), except 64-bit unsigned which stays unsigned (like C).
template <typename L, typename R>
struct PromotedType {
    static auto select() {
        if constexpr (std::is_same_v<L, double> || std::is_same_v<R, double>)
            return double{};
        else if constexpr (std::is_same_v<L, float> || std::is_same_v<R, float>)
            return float{};
        else if constexpr (std::is_same_v<L, R>)
            return L{};
        else if constexpr (std::is_signed_v<L> == std::is_signed_v<R>)
            return std::conditional_t<(sizeof(L) >= sizeof(R)), L, R>{};
        else if constexpr (std::is_unsigned_v<L> ? sizeof(L) == 8 : sizeof(R) == 8)
            return uint64_t{};
        else
            return int64_t{};
    }
    using type = decltype(select());
};

/// @brief LPC truthiness: object handles are true only while their object is alive.
/// Non-zero numbers and other non-void values are true.
static bool is_truthy(const SValue& value) {
    return std::visit([](const auto& v) -> bool {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_arithmetic_v<T>)
            return v != 0;
        else if constexpr (std::is_same_v<T, std::monostate>)
            return false;
        else if constexpr (std::is_same_v<T, LpcObject::HandleType>)
            return !v.expired();
        else
            return true;
    }, value);
}

static bool is_string(const SValue& value) {
    return std::holds_alternative<TemporaryString>(value) || std::holds_alternative<LpcStringRef>(value) ||
        std::holds_alternative<LpcBufferRef>(value);
}

lpcvm::Execution& lpcvm::Execution::as_with(ObjectState* self, std::size_t offset) {
    self_ = self;
    var0_ = offset;
    return *this;
}

/**
 * @brief Execution::execute() implementation (a token-threaded virtual machine)
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
int lpcvm::Execution::execute (ReturnStack& control, DataStack& stack, std::size_t num_args) {
    // Keep the opcode address separate from pc, which advances over operands and jumps.
    const uint8_t* instruction_pc = start_;
    try {
        if (!length() || start_ >= end_) {
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
            if (pc < end_) { instruction_pc = pc; goto *dispatch_table[*pc++]; } else { pc = nullptr; goto do_F_RETURN; }
        #define VM_DISPATCH_EXTENDED() \
            if (pc < end_) { instruction_pc = pc; goto *dispatch_table[*pc++]; } else { pc = nullptr; goto do_F_RETURN; }
#else
        // Computed goto is not supported, using switch-case instead
        #define VM_LOOP_START() \
            static uint8_t force_return[] = { F_RETURN, 0 }; \
            do { \
                if (pc != force_return) instruction_pc = pc; \
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
        std::string string_accumulator;
        bool string_active = false;
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
            int32_t value = prog_->const_pool()->fetch_numeric<int32_t>(oparg);
            stack.push(SValue(value));
            VM_DISPATCH();
        }

        VM_HANDLER(F_CONST_I8) {
            oparg = (extended_arg << 8) | *pc++;
            int64_t value = prog_->const_pool()->fetch_numeric<int64_t>(oparg);
            stack.push(SValue(value));
            VM_DISPATCH();
        }

        VM_HANDLER(F_CONST_U4) {
            oparg = (extended_arg << 8) | *pc++;
            uint32_t value = prog_->const_pool()->fetch_numeric<uint32_t>(oparg);
            stack.push(SValue(value));
            VM_DISPATCH();
        }

        VM_HANDLER(F_CONST_U8) {
            oparg = (extended_arg << 8) | *pc++;
            uint64_t value = prog_->const_pool()->fetch_numeric<uint64_t>(oparg);
            stack.push(SValue(value));
            VM_DISPATCH();
        }

        VM_HANDLER(F_CONST_F4) {
            oparg = (extended_arg << 8) | *pc++;
            float value = prog_->const_pool()->fetch_numeric<float>(oparg);
            stack.push(SValue(value));
            VM_DISPATCH();
        }

        VM_HANDLER(F_CONST_F8) {
            oparg = (extended_arg << 8) | *pc++;
            double value = prog_->const_pool()->fetch_numeric<double>(oparg);
            stack.push(SValue(value));
            VM_DISPATCH();
        }

        VM_HANDLER(F_CONST_STR) {
            oparg = (extended_arg << 8) | *pc++;
            if (!prog_ || !prog_->const_pool())
                throw std::runtime_error("Unable to fetch string constant without a constant pool");
            {
                auto value = prog_->const_pool()->fetch_string(oparg);
                stack.push(SValue(value));
            }
            VM_DISPATCH();
        }

        VM_HANDLER(F_LOAD_GLOBAL) {
            oparg = (extended_arg << 8) | *pc++;
            if (!self_)
                throw std::runtime_error("Global variable access requires an object state");
            stack.push((*self_)[var0_ + oparg]);
            VM_DISPATCH();
        }

        VM_HANDLER(F_STORE_GLOBAL) {
            oparg = (extended_arg << 8) | *pc++;
            if (!self_)
                throw std::runtime_error("Global variable access requires an object state");
            if (std::holds_alternative<TemporaryString>(stack.top()) || std::holds_alternative<LpcBufferRef>(stack.top())) {
                if (!fabric_)
                    throw std::runtime_error("Unable to perform string operation without a fabric");
                (*self_)[var0_ + oparg] = fabric_->make_shared_string(stack.top_string());
            } else {
                (*self_)[var0_ + oparg] = stack.top();
            }
            VM_DISPATCH();
        }

        VM_HANDLER(F_LOAD_LOCAL) {
            oparg = (extended_arg << 8) | *pc++;
            stack.push_copy(fp + oparg);
            VM_DISPATCH();
        }

        VM_HANDLER(F_STORE_LOCAL) {
            oparg = (extended_arg << 8) | *pc++;
            if (stack.empty())
                throw std::runtime_error("Evaluation stack is empty");
            stack.assign_copy(fp + oparg, stack.size() - 1);
            VM_DISPATCH();
        }

        VM_HANDLER(F_STORE_LOCAL_BUFFER) {
            oparg = (extended_arg << 8) | *pc++;
            if (!fabric_)
                throw std::runtime_error("Unable to perform string operation without a fabric");
            stack.assign_value(fp + oparg, *fabric_);
            VM_DISPATCH();
        }

        VM_HANDLER(F_UNARY_OP) {
            oparg = (extended_arg << 8) | *pc++;
            auto& value = stack.top();
            result = std::visit([&](const auto& val) -> SValue {
                using ValType = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<ValType, LpcObject::HandleType>) {
                    if (oparg == oparg::NOT)
                        return SValue(val.expired());
                }
                if constexpr (std::is_arithmetic_v<ValType>) {
                    // INC/DEC preserve the operand type, so no typed constant is needed
                    if (oparg == oparg::INC)
                        return SValue(static_cast<ValType>(val + 1));
                    if (oparg == oparg::DEC)
                        return SValue(static_cast<ValType>(val - 1));
                }
                if constexpr (std::is_integral_v<ValType>) {
                    switch (oparg) {
                        case oparg::NOT:
                            return SValue(!val);
                        case oparg::NEG:
                            return SValue(-val);
                        case oparg::BITWISE_NOT:
                            return SValue(~val);
                        default:
                            throw std::runtime_error("Invalid unary operation");
                    }
                } else {
                    throw std::runtime_error("Unsupported unary operand type");
                }
            }, value);
            stack.assign(stack.size() - 1, result);
            VM_DISPATCH();
        }

        VM_HANDLER(F_BINARY_OP) {
            oparg = (extended_arg << 8) | *pc++; // fetch the binary operation code
            auto sp = stack.size();
            if (sp < 2)
                throw std::runtime_error("Evaluation stack underflow");
            auto& rhs = stack[sp - 1];
            auto& lhs = stack[sp - 2];
            if (oparg == oparg::ADD && is_string(lhs) && is_string(rhs)) {
                stack.concat_top_strings();
                VM_DISPATCH();
            }
            result = std::visit([&](const auto& lhs_value, const auto& rhs_value) -> SValue {
                using LhsType = std::decay_t<decltype(lhs_value)>;
                using RhsType = std::decay_t<decltype(rhs_value)>;
                if constexpr (std::is_arithmetic_v<LhsType> && std::is_arithmetic_v<RhsType>) {
                    if (oparg == oparg::DIV && rhs_value == 0)
                        throw std::runtime_error("Division by zero");
                }
                if constexpr (std::is_arithmetic_v<LhsType> && std::is_arithmetic_v<RhsType>) {
                    using T = typename PromotedType<LhsType, RhsType>::type;
                    const T a = static_cast<T>(lhs_value);
                    const T b = static_cast<T>(rhs_value);
                    switch (oparg) {
                        case oparg::ADD:
                            return SValue(static_cast<T>(a + b));
                        case oparg::SUB:
                            return SValue(static_cast<T>(a - b));
                        case oparg::MUL:
                            return SValue(static_cast<T>(a * b));
                        case oparg::DIV:
                            return SValue(static_cast<T>(a / b));
                        case oparg::EQUAL:
                            return SValue(a == b);
                        case oparg::NOT_EQUAL:
                            return SValue(a != b);
                        case oparg::LESS:
                            return SValue(a < b);
                        case oparg::LESS_EQUAL:
                            return SValue(a <= b);
                        case oparg::GREATER:
                            return SValue(a > b);
                        case oparg::GREATER_EQUAL:
                            return SValue(a >= b);
                        default:
                            break;
                    }
                    if constexpr (std::is_integral_v<T>) {
                        switch (oparg) {
                            case oparg::MOD:
                                return SValue(static_cast<T>(a % b));
                            case oparg::BITWISE_AND:
                                return SValue(static_cast<T>(a & b));
                            case oparg::BITWISE_OR:
                                return SValue(static_cast<T>(a | b));
                            case oparg::BITWISE_XOR:
                                return SValue(static_cast<T>(a ^ b));
                            case oparg::LSH:
                                return SValue(static_cast<LhsType>(lhs_value << rhs_value)); // shifts keep the lhs type
                            case oparg::RSH:
                                return SValue(static_cast<LhsType>(lhs_value >> rhs_value));
                            default:
                                break;
                        }
                    }
                    throw std::runtime_error("Invalid binary operation");
                }
                else if constexpr (std::is_same_v<LhsType, LpcObject::HandleType> &&
                                   std::is_same_v<RhsType, LpcObject::HandleType>) {
                    switch (oparg) {
                        case oparg::EQUAL:
                            return SValue(lhs_value == rhs_value);
                        case oparg::NOT_EQUAL:
                            return SValue(!(lhs_value == rhs_value));
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

        VM_HANDLER(F_STR_BEGIN)
        VM_HANDLER(F_STR_APPEND) {
            const bool begin = pc[-1] == F_STR_BEGIN;
            if (pc >= end_)
                throw std::runtime_error("Missing string accumulator argument");
            oparg = (extended_arg << 8) | *pc++;
            if (oparg != 0)
                throw std::runtime_error("Invalid string accumulator argument");
            if (begin == string_active)
                throw std::runtime_error("Invalid string accumulator state");
            if (stack.size() <= fp)
                throw std::runtime_error("String accumulator operand underflow");
            const auto operand = stack.top_string();
            constexpr auto max = std::numeric_limits<uint32_t>::max();
            if (operand.size() > max || string_accumulator.size() > max - operand.size())
                throw std::length_error("String accumulator exceeds its representable size");
            string_accumulator.append(operand);
            stack.pop();
            string_active = true;
            VM_DISPATCH();
        }

        VM_HANDLER(F_STR_FINISH) {
            if (pc >= end_)
                throw std::runtime_error("Missing string accumulator argument");
            oparg = (extended_arg << 8) | *pc++;
            if (oparg != 0)
                throw std::runtime_error("Invalid string accumulator argument");
            if (!string_active)
                throw std::runtime_error("String accumulator is inactive");
            stack.push_temp_string(string_accumulator);
            string_accumulator.clear();
            string_active = false;
            VM_DISPATCH();
        }

        VM_HANDLER(F_STR_FINISH_LOCAL) {
            if (pc >= end_)
                throw std::runtime_error("Missing string accumulator argument");
            oparg = (extended_arg << 8) | *pc++;
            if (!string_active)
                throw std::runtime_error("String accumulator is inactive");
            if (!fabric_)
                throw std::runtime_error("Unable to perform string operation without a fabric");
            stack.swap_into_buffer(fp + oparg, string_accumulator, *fabric_);
            string_active = false;
            stack.push_copy(fp + oparg);
            VM_DISPATCH();
        }

        VM_HANDLER(F_PUSH_SELF) {
            if (pc >= end_)
                throw std::runtime_error("Missing self argument");
            oparg = (extended_arg << 8) | *pc++;
            stack.push(LpcObject::HandleType(obj_ref_)); // non-owning reference to self
            VM_DISPATCH();
        }

        VM_HANDLER(F_PUSH_FUNC) {
            oparg = (extended_arg << 8) | *pc++;
            stack.push(oparg); // push the function index onto the stack
            VM_DISPATCH();
        }

        VM_HANDLER(F_CALL) {
            if (string_active)
                throw std::runtime_error("Cannot call with an active string accumulator");
            oparg = (extended_arg << 8) | *pc++; // number of items on the stack for this call (func_index + arguments)
            if (oparg < 1)
                throw std::runtime_error("Invalid call with no function index on the stack");
            auto func_index = std::get<uint32_t>(stack[stack.size() - oparg]);
            auto binding = prog_->get_method(func_index);
            control.push(pc, fp, var0_, start_, end_, obj_ref_); // save the current execution context on the return stack
            fp = stack.size() - oparg + 1; // set the frame pointer for the new function call (fp -> first argument)
            pc = start_ = prog_->bytecode() + binding.bytecode_offset;
            end_ = start_ + binding.bytecode_length;
            VM_DISPATCH();
        }

        VM_HANDLER(F_JUMP) {
            oparg = (extended_arg << 8) | *pc++;
            pc += static_cast<int32_t>(oparg);
            if (pc < start_ || pc > end_)
                throw std::runtime_error("Jump target out of bounds");
            VM_DISPATCH();
        }

        VM_HANDLER(F_JUMP_IF_FALSE) {
            oparg = (extended_arg << 8) | *pc++;
            bool truth = is_truthy(stack.top());
            stack.pop();
            if (!truth) {
                pc += static_cast<int32_t>(oparg);
                if (pc < start_ || pc > end_)
                    throw std::runtime_error("Jump target out of bounds");
            }
            VM_DISPATCH();
        }

        VM_HANDLER(F_JUMP_IF_FALSE_OR_POP)
        VM_HANDLER(F_JUMP_IF_TRUE_OR_POP) {
            const bool jump_if_true = (pc[-1] == F_JUMP_IF_TRUE_OR_POP);
            oparg = (extended_arg << 8) | *pc++;
            bool truth = is_truthy(stack[stack.size() - 1]);
            if (truth == jump_if_true) {
                pc += static_cast<int32_t>(oparg); // keep the value on the stack as the result
                if (pc < start_ || pc > end_)
                    throw std::runtime_error("Jump target out of bounds");
            }
            else
                stack.pop();
            VM_DISPATCH();
        }

        VM_HANDLER(F_RETURN) {
            if (string_active)
                throw std::runtime_error("Cannot return with an active string accumulator");
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
            var0_ = control.top().vbp;
            start_ = control.top().start;
            end_ = control.top().end;
            obj_ref_ = control.top().obj.lock();
            control.pop();
            VM_DISPATCH();
        }

        VM_LOOP_END()
        return 0;

    invalid_opcode:
        throw std::runtime_error("Invalid opcode");

    } catch (const std::overflow_error& error) {
        if (dynamic_cast<const lpcvm::Exception<std::overflow_error>*>(&error))
            throw;
        throw lpcvm::Exception(error, prog_, instruction_pc, obj_ref_);
    } catch (const std::runtime_error& error) {
        if (dynamic_cast<const lpcvm::Exception<std::runtime_error>*>(&error))
            throw;
        throw lpcvm::Exception(error, prog_, instruction_pc, obj_ref_);
    } catch (const std::length_error& error) {
        if (dynamic_cast<const lpcvm::Exception<std::length_error>*>(&error))
            throw;
        throw lpcvm::Exception(error, prog_, instruction_pc, obj_ref_);
    } catch (const std::out_of_range& error) {
        if (dynamic_cast<const lpcvm::Exception<std::out_of_range>*>(&error))
            throw;
        throw lpcvm::Exception(error, prog_, instruction_pc, obj_ref_);
    } catch (const std::invalid_argument& error) {
        if (dynamic_cast<const lpcvm::Exception<std::invalid_argument>*>(&error))
            throw;
        throw lpcvm::Exception(error, prog_, instruction_pc, obj_ref_);
    } catch (const std::bad_variant_access& error) {
        if (dynamic_cast<const lpcvm::Exception<std::bad_variant_access>*>(&error))
            throw;
        throw lpcvm::Exception(error, prog_, instruction_pc, obj_ref_);
    }

#undef VM_LOOP_START
#undef VM_LOOP_END
#undef VM_HANDLER
#undef VM_DISPATCH
#undef VM_HAS_COMPUTED_GOTO
} // Execution::execute()
