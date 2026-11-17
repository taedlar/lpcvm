// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "lpc_types.hpp"
#include "vm_svalue.hpp"
#include <cassert>
#include <functional>

std::size_t LpcMapping::string_hash(std::string_view value) noexcept {
    return std::hash<std::string_view>{}(value);
}

std::size_t LpcMapping::hash(const lpcvm::svalue_t& key) noexcept {
    assert(!key.value.valueless_by_exception());
    if (key.value.valueless_by_exception())
        return 0;

    const auto value_hash = std::visit([](const auto& value) noexcept -> std::size_t {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
        } else if constexpr (std::is_arithmetic_v<T>) {
            // std::hash also gives +0 and -0 the same hash.
            return std::hash<T>{}(value);
        } else if constexpr (std::is_same_v<T, LpcStringRef>) {
            return value ? string_hash(value->view()) : 0;
        } else if constexpr (std::is_same_v<T, LpcBufferRef>) {
            return value ? string_hash(value->data) : 0;
        } else if constexpr (std::is_same_v<T, TemporaryString>) {
            // The descriptor alone cannot resolve the stack-owned bytes.
            assert(false && "Use string_hash with the temporary string's stack-owned bytes");
            return 0;
        } else if constexpr (std::is_same_v<T, LpcObject::HandleType>) {
            // Keep the identity even after expiration; locking would lose it.
            return std::hash<const void*>{}(value.ptr);
        } else {
            // Arrays and mappings have reference semantics.
            return std::hash<const void*>{}(value ? std::addressof(*value) : nullptr);
        }
    }, key.value);
    // String representations share a hash, including resolved temporary strings.
    if (std::holds_alternative<LpcStringRef>(key.value) ||
        std::holds_alternative<LpcBufferRef>(key.value))
        return value_hash;
    // Mapping keys retain the numeric variant type, as constant-pool keys do.
    return value_hash ^ (key.value.index() + std::size_t{0x9e3779b9}
                         + (value_hash << 6) + (value_hash >> 2));
}

LpcMapping::~LpcMapping() {
    // Clean up hash table and the mapping entries
}
