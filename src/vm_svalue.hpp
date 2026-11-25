// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <type_traits>
#include <variant>
#include "lpc_types.hpp"

namespace lpcvm { class DataStack; }

struct TemporaryString {
private:
    friend class lpcvm::DataStack; // allow DataStack to construct TemporaryString instances
    TemporaryString() = default;
    uint32_t offset;
    uint32_t length;
};

/**
 * @brief Represents a value that can hold any of the types allowed to be
 * pushed onto the virtual machine's data stack.
 *
 * Uses std::variant to provide type-safe storage and access.
 *
 * Owning LPC strings are represented by LpcStringRef, interned through a
 * ManagedRuntime. Copying a value retains its string, including in ObjectState.
 *
 * LPC reference semantics are implemented using a lpcvm::SharedPtr variant
 * for mappings and arrays, and a generational handle variant for objects.
 *
 * Current implementation keeps the size of SValue at 16 bytes on 64-bit
 * architectures: 8 bytes for std::variant index and 8 bytes for the
 * value storage.
 */
using SValue = std::variant<
    std::monostate,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t,
    float,
    double,
    TemporaryString, // stack-owned temporary string
    lpcvm::ObjectRef, // resource descriptor
    LpcStringRef,
    LpcBufferRef,
    LpcMappingRef,
    LpcArrayRef,
    LpcObject::HandleType
>;

namespace lpcvm {
struct svalue_t {
    SValue value;
};
struct svalue_pair_t {
    SValue first;
    svalue_t second;
};

/**
 * @brief Compare two SValue instances.
 *
 * Returns 0 if the values are equal, a negative value if a < b, and a positive value if a > b.
 * The comparison logic should handle all types stored in SValue appropriately.
 *
 * Different types are ordered by variant index, except string representations.
 * Numeric values of the same type use natural ordering; signed zeros are equal.
 * NaNs sort after other values of their type and are ordered by representation.
 * String types (LpcStringRef and LpcBufferRef) are compared by their content.
 * Reference types (LpcMappingRef, LpcArrayRef, and LpcObject::HandleType) are compared by their identity.
 * Null string references compare equal across representations and before non-null strings.
 * Expired object handles retain their identity. Valueless variants compare equal
 * to each other and sort after supported values.
 *
 * TemporaryString comparison is not supported (always returns -1) and should be handled separately.
 */
int compare_svalues(const SValue& a, const SValue& b);

} // namespace lpcvm
