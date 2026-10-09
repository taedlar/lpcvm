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

/// \brief The enum definition for opcodes using the X-macro technique. This enum must be 0-based,
/// with the first opcode having the value 0.
enum Opcode: uint8_t {
#define AS_ENUM(op) op,
    ALL_OPCODES(AS_ENUM)
#undef AS_ENUM
    INVALID_VALUE // start of invalid opcode, this must come last
};
