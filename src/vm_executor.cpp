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
void BytecodeView::execute() {
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
        if (pc < end) goto *dispatch_table[*pc++]; else goto end_of_bytecode;
    #define VM_DISPATCH_EXTENDED() \
        if (pc < end) goto *dispatch_table[*pc++]; else goto end_of_bytecode;
#else
    // Computed goto is not supported, using switch-case instead
    #define VM_LOOP_START() \
        while (pc < end) { \
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

    const uint8_t *pc = bytecode_, *end = bytecode_ + size_;
    uint32_t extended_arg = 0; // holds the extended argument for the next opcode (CPython style)
    VM_LOOP_START()

    VM_HANDLER(F_RETURN) {
        uint32_t result = extended_arg | *pc++;
        return;
    }

    VM_HANDLER(F_EXTENDED_ARG) {
        extended_arg = (extended_arg << 8) | *pc++;
        VM_DISPATCH_EXTENDED();
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
