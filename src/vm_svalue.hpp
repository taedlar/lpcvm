// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <type_traits>
#include <variant>
#include "lpc_types.hpp"

using ArenaStringView = std::pair<uint32_t, uint32_t>; // first: offset, second: length in the arena

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

/**
 * @brief Represents a value that can hold any of the types allowed to be
 * pushed onto the virtual machine's evaluation stack, as defined in
 * SValueVariant.
 *
 * Inherits from std::variant to provide type-safe storage and access.
 *
 * LPC strings must be interned in the arena and are represented using
 * ArenaStringView, which stores the offset and length of the string in
 * the arena.
 *
 * LPC reference semantics are implemented using a LpcSharedPtr variant
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
    ArenaStringView,
    LpcMappingRef,
    LpcArrayRef,
    LpcObjectRef
>;
