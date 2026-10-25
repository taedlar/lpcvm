// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once

#include <vector>
#include <cstdint>
#include "ast_node.hpp"
#include "vm_opcodes.hpp"

namespace lpcvm {

    class BytecodeGenerator {
    private:
        std::vector<uint8_t>& bytecode_;
        enum Context {
            STATEMENT,
            EXPRESSION
        } context_;

    public:
        BytecodeGenerator(std::vector<uint8_t>& bytecode)
            : bytecode_(bytecode), context_(STATEMENT) {}

        /**
         * @brief Emits a bytecode instruction with its argument.
         *
         * We follow the "wordcode" convention as in Python's bytecode, where each
         * instruction consists of an opcode (uint8_t) followed by its argument (uint8_t).
         */
        BytecodeGenerator& emit(uint8_t op, uint32_t index = 0) {
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
            return *this;
        }

        /**
         * @brief Emits bytecode for a given AST statement.
         *
         * This function dispatches the statement to the appropriate bytecode
         * emission logic based on its type.
         */
        BytecodeGenerator& emit_statement(const ASTNode& stmt);

        BytecodeGenerator& emit_expression(const ASTNode& expr);
    };

    class FunctionBuilder: public BytecodeGenerator {
    public:
        FunctionBuilder(std::vector<uint8_t>& bytecode)
            : BytecodeGenerator(bytecode) {}
        FunctionBuilder& emit_body(const AST::Block& body, std::size_t num_formal_params = 0);
    };

} // namespace lpcvm
