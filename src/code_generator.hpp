// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once

#include <vector>
#include <cstdint>
#include "ast_node.hpp"
#include "vm_opcodes.hpp"

namespace lpcvm {

    class CodeGenerator {
    private:
        std::vector<uint8_t>& bytecode_;
        void grow_bytecode(std::size_t length);

        enum Context {
            STATEMENT,
            EXPRESSION
        } context_;

    public:
        CodeGenerator(std::vector<uint8_t>& bytecode)
            : bytecode_(bytecode), context_(STATEMENT) {}

        /**
         * @brief Emits a bytecode instruction with its argument.
         *
         * We follow the "wordcode" convention as in Python's bytecode, where
         * each instruction consists of an opcode (uint8_t) followed by its
         * argument (uint8_t). The argument can be extended to 16/24/32 bits
         * using the F_EXTENDED_ARG opcode if it exceeds one byte. The design
         * allows for flexible argument sizes while keeping the common case
         * of single-byte arguments efficient. It also benefits benefit from
         * CPU cache locality for the instructions.
         *
         * Common case: 2 bytes (8-bit opcode followed by 8-bit argument).
         * Extended case:
         *  4 bytes (F_EXTENDED_ARG + 8-bit opcode + 16-bit argument)
         *  6 bytes (F_EXTENDED_ARG x2 + 8-bit opcode + 24-bit argument)
         *  8 bytes (F_EXTENDED_ARG x3 + 8-bit opcode + 32-bit argument)
         *
         * Instructions preserve two-byte boundaries when the existing bytecode
         * has even length. Capacity grows geometrically in 64-byte multiples;
         * this does not guarantee cache-line alignment of the underlying storage.
         *
         * @param op The opcode to emit.
         * @param index The argument for the opcode, defaulting to 0.
         */
        CodeGenerator& emit(uint8_t op, uint32_t index = 0) {
            const std::size_t length = index > 0x00ffffff ? 8 :
                index > 0x0000ffff ? 6 : index > 0x000000ff ? 4 : 2;
            if (length > bytecode_.capacity() - bytecode_.size())
                grow_bytecode(length);
            if (index > 0x00ffffff) {
                bytecode_.push_back(F_EXTENDED_ARG);
                bytecode_.push_back((index >> 24) & 0xff);
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
        CodeGenerator& emit_statement(const ASTNode& stmt);

        CodeGenerator& emit_expression(const ASTNode& expr);

        CodeGenerator& emit_block(const AST::Block& block, std::size_t num_formal_params = 0);
    };

    class FunctionBuilder: public CodeGenerator {
    public:
        FunctionBuilder(std::vector<uint8_t>& bytecode)
            : CodeGenerator(bytecode) {}
        FunctionBuilder& emit_body(const AST::Block& body, std::size_t num_formal_params = 0);
    };

} // namespace lpcvm
