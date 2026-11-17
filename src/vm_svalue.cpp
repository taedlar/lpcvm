// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_svalue.hpp"
#include <cmath>
#include <cstring>
#include <functional>

int lpcvm::compare_svalues(const SValue& a, const SValue& b) {
    if (std::holds_alternative<TemporaryString>(a) ||
        std::holds_alternative<TemporaryString>(b))
        return -1;

    const auto is_string = [](const SValue& value) {
        return std::holds_alternative<LpcStringRef>(value) ||
               std::holds_alternative<LpcBufferRef>(value);
    };
    if (is_string(a) && is_string(b)) {
        const auto bytes = [](const SValue& value) -> std::optional<std::string_view> {
            if (const auto* ref = std::get_if<LpcStringRef>(&value)) {
                if (*ref)
                    return (*ref)->view();
            } else if (const auto& ref = std::get<LpcBufferRef>(value)) {
                return ref->data;
            }
            return std::nullopt;
        };
        const auto lhs = bytes(a);
        const auto rhs = bytes(b);
        return (lhs > rhs) - (lhs < rhs);
    }

    // Preserve type-sensitive mapping-key equality and hash consistency.
    if (a.index() != b.index())
        return a.index() < b.index() ? -1 : 1;
    if (a.valueless_by_exception())
        return 0;

    return std::visit([&](const auto& lhs) -> int {
        using T = std::decay_t<decltype(lhs)>;
        const auto& rhs = std::get<T>(b);
        if constexpr (std::is_same_v<T, std::monostate>) {
            return 0;
        } else if constexpr (std::is_arithmetic_v<T>) {
            if constexpr (std::is_floating_point_v<T>) {
                const bool lhs_nan = std::isnan(lhs);
                const bool rhs_nan = std::isnan(rhs);
                if (lhs_nan != rhs_nan)
                    return lhs_nan ? 1 : -1;
                if (lhs_nan) {
                    // Order NaNs by representation, without equating them to numbers.
                    using Bits = std::conditional_t<sizeof(T) == sizeof(uint32_t),
                                                    uint32_t, uint64_t>;
                    Bits lhs_bits, rhs_bits;
                    std::memcpy(&lhs_bits, &lhs, sizeof(T));
                    std::memcpy(&rhs_bits, &rhs, sizeof(T));
                    return (lhs_bits > rhs_bits) - (lhs_bits < rhs_bits);
                }
            }
            return (lhs > rhs) - (lhs < rhs);
        } else if constexpr (std::is_same_v<T, LpcObject::HandleType>) {
            return (rhs < lhs) - (lhs < rhs);
        } else if constexpr (std::is_same_v<T, TemporaryString>) {
            return -1;
        } else {
            const auto* lhs_ptr = lhs ? std::addressof(*lhs) : nullptr;
            const auto* rhs_ptr = rhs ? std::addressof(*rhs) : nullptr;
            const std::less<decltype(lhs_ptr)> less;
            return less(rhs_ptr, lhs_ptr) - less(lhs_ptr, rhs_ptr);
        }
    }, a);
}
