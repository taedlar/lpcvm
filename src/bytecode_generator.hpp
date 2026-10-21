// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once

#include <vector>
#include <cstdint>
#include "ast_node.hpp"
#include "vm_opcodes.hpp"

namespace lpcvm {

    class BytecodeGenerator {
    public:
        BytecodeGenerator(std::vector<uint8_t>& bytecode)
            : bytecode_(bytecode) {}

        /**
         * @brief Emits a bytecode instruction with its argument.
         *
         * We follow the "wordcode" convention as in Python's bytecode, where each
         * instruction consists of an opcode followed by its argument.
         */
        inline void emit(uint8_t op, uint8_t oparg) {
            bytecode_.push_back(op);
            bytecode_.push_back(oparg);
        }

        inline void emit(uint8_t op) {
            bytecode_.push_back(op);
            bytecode_.push_back(0); // default argument for single-byte emit
        }

        inline void emit(uint8_t op, uint32_t index) {
            if (index > 0x00ffffff) {
                bytecode_.push_back(F_EXTENDED_ARG); // hypothetical extended arg opcode
                bytecode_.push_back((index >> 24) & 0xff); // high byte of extended argument
            }
            if (index > 0x0000ffff) {
                bytecode_.push_back(F_EXTENDED_ARG);
                bytecode_.push_back((index >> 16) & 0xff);
            }
            if (index > 0x000000ff) {
                bytecode_.push_back(F_EXTENDED_ARG);
                bytecode_.push_back((index >> 8) & 0xff);
            }
            bytecode_.push_back(op);
            bytecode_.push_back(index & 0xff);
        }

    private:
        std::vector<uint8_t>& bytecode_;
    };

    class FunctionBuilder: public BytecodeGenerator {
    public:
        FunctionBuilder(std::vector<uint8_t>& bytecode)
            : BytecodeGenerator(bytecode) {}
        void emit_body(const ASTNode* body) const;
    };

} // namespace lpcvm
