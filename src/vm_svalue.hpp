// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <type_traits>
#include <variant>
#include "lpc_types.hpp"

/**
 * @brief Get the index of type T in the variant Variant.
 * Returns std::variant_size_v<Variant> if not found.
 */
template <typename Variant, typename T, std::size_t I = 0>
constexpr std::size_t variant_index() {
    if constexpr (I >= std::variant_size_v<Variant>) {
        return std::variant_size_v<Variant>; // Type not found
    } else {
        if constexpr (std::is_same_v<std::variant_alternative_t<I, Variant>, T>) {
            return I;
        } else {
            return variant_index<Variant, T, I + 1>();
        }
    }
}

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
    LpcStringRef,
    LpcMappingRef,
    LpcArrayRef,
    LpcObject::HandleType
>;
