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
    X(F_RETURN) \
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

/// \brief The enum definition for opcodes using the X-macro technique.
/// This enum must be 0-based, with the first opcode having the value 0.
enum Opcode: uint8_t {
#define AS_ENUM(op) op,
    ALL_OPCODES(AS_ENUM)
#undef AS_ENUM
    INVALID_VALUE // start of invalid opcode, this must come last
};

namespace oparg {

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
    ASSIGN,
    INVALID_BINARY_OPARG
};

} // namespace oparg
