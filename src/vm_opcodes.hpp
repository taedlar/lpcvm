// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>

/// \brief The X-macro to list all opcodes for the definition of the enum Opcode and dispatch labels
/// Each opcode always consume the following uint8_t as argument.
/// The F_EXTENDED_ARG opcode is used to extend the argument for the next opcode (CPython style).
/// This allows for arguments larger than 8 bits by using one or more F_EXTENDED_ARG opcodes before
/// the actual opcode.
#define ALL_OPCODES(X) \
    X(F_EXTENDED_ARG) \
    X(F_PUSH_N) \
    X(F_POP_N) \
    X(F_CONST_I4) \
    X(F_CONST_I8) \
    X(F_CONST_U4) \
    X(F_CONST_U8) \
    X(F_CONST_F4) \
    X(F_CONST_F8) \
    X(F_CONST_STR) \
    X(F_LOAD_GLOBAL) \
    X(F_STORE_GLOBAL) \
    X(F_LOAD_LOCAL) \
    X(F_STORE_LOCAL) \
    X(F_BIN_OP) \
    X(F_PUSH_FUNC) \
    X(F_CALL) \
    X(F_JUMP) \
    X(F_JUMP_IF_FALSE) \
    X(F_RETURN) \

/// F_JUMP adds its argument to the program counter (relative to the next instruction).
/// F_JUMP_IF_FALSE pops the top of the stack and jumps only if the value is false (zero/void).
/// \brief The enum definition for opcodes using the X-macro technique.
/// This enum must be 0-based, with the first opcode having the value 0.
enum Opcode: uint8_t {
#define AS_ENUM(op) op,
    ALL_OPCODES(AS_ENUM)
#undef AS_ENUM
    INVALID_VALUE // start of invalid opcode, this must come last
};

namespace oparg {

enum UnaryOpArg {
    NOT,
    NEG,
    BITWISE_NOT,
    INVALID_UNARY_OPARG
};

/// @brief The enum definition for binary operation arguments used by the
/// F_BIN_OP opcode. The consolidated binary operation argument follows
/// the design style of Python's bytecode.
enum BinaryOpArg {
    COMMA,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    BITWISE_AND,
    BITWISE_OR,
    BITWISE_XOR,
    LSH,
    RSH,
    LOGICAL_AND,
    LOGICAL_OR,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    INVALID_BINARY_OPARG
};

enum AssignOpArg {
    ASSIGN,
    ASSIGN_ADD,
    ASSIGN_SUB,
    ASSIGN_MUL,
    ASSIGN_DIV,
    ASSIGN_MOD,
    ASSIGN_AND,
    ASSIGN_OR,
    ASSIGN_XOR,
    ASSIGN_PREFIX_INC,
    ASSIGN_PREFIX_DEC,
    ASSIGN_SUFFIX_INC,
    ASSIGN_SUFFIX_DEC,
    INVALID_ASSIGN_OPARG
};

} // namespace oparg
