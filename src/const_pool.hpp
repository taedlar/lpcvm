// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>
#include "lpc_types.hpp"

struct ManagedRuntime;

/**
 * @brief A flattened array of constant values, used by the VM runtime to
 * retrieve constants. Fabric-scoped shared strings are interned through the
 * ManagedRuntime, eliminating the need for a separate string buffer.
 *
 * Numeric constants are stored in SlotType units, ensuring alignment and
 * relocatability.
 *
 * Numeric values occupy sizeof(T)/sizeof(SlotType) consecutive slots.
 * String constants are represented by marked indices into a vector of
 * fabric-scoped LpcStringRef instances.
 *
 * Type of value is not stored explicitly; it is encoded with opcodes that
 * loads the constant from the pool. The fabric must outlive any ConstTable
 * that adds constants; a frozen pool retains its strings independently.
 */
class ConstPool {
public:
    explicit ConstPool(ManagedRuntime& fabric) : fabric_(fabric) {}
    ConstPool(const ConstPool&) = default;
    ~ConstPool() = default;

    using SlotType = uint32_t; // flattened unit of storage
    using IndexType = uint32_t; // runtime index (this is the max width, extended from 8-bit via F_EXTENDED_ARG in bytecode)
    constexpr static IndexType INVALID_INDEX = std::numeric_limits<IndexType>::max();

    // Special marker to distinguish string ref indices from numeric pool indices.
    // String indices are negative when cast to int32_t, but stored as high bits.
    constexpr static uint32_t STRING_REF_MARKER = 0x80000000;
    constexpr static uint32_t STRING_REF_INDEX_MASK = 0x7FFFFFFF;

    /**
     * @brief Read a numeric constant at slot @p index (alignment-safe).
     * Specialized fetch for numeric types.
     */
    template<typename T>
    T fetch_numeric(IndexType index) const {
        static_assert(std::is_arithmetic_v<T>, "use fetch_string() for strings");
        static_assert(sizeof(T) % sizeof(SlotType) == 0);
        if (index + (sizeof(T) / sizeof(SlotType) - 1) >= pool_.size()) {
            throw std::out_of_range("ConstPool::fetch_numeric: index out of range");
        }
        T value;
        std::memcpy(&value, &pool_[index], sizeof(T));
        return value;
    }

    /**
     * @brief Read a fabric-scoped string constant at slot @p index.
     * The pool holds an owning reference to each interned string.
     */
    LpcStringRef fetch_string(IndexType index) const;

    /**
     * @brief Check if an index refers to a string constant.
     */
    bool is_string_index(IndexType index) const {
        return (index & STRING_REF_MARKER) != 0;
    }

    size_t slot_count() const {
        return pool_.size();
    }

    size_t string_count() const {
        return string_refs_.size();
    }

private:
    friend class ConstTable;
    ManagedRuntime& fabric_;                       // borrowed for interning by ConstTable
    std::vector<SlotType> pool_;                   // numeric constants at O(1) access
    std::vector<LpcStringRef> string_refs_;
};


/**
 * @brief A compile-time constant table that extends ConstPool with efficient
 * lookup and addition of constants. Integrates with a ManagedRuntime to intern
 * strings as fabric-scoped LpcStringRef values.
 */
class ConstTable {
public:
    explicit ConstTable(ManagedRuntime& fabric) : store_(fabric) {}
    ~ConstTable() = default;

    using ValueType = std::variant<int32_t, uint32_t, int64_t, uint64_t, float, double, std::string_view>;
    using NumericType = std::variant<int32_t, uint32_t, int64_t, uint64_t, float, double>;

    ConstPool::IndexType INVALID_INDEX = ConstPool::INVALID_INDEX;

    ConstPool::IndexType find(const ValueType& value) const;

    /**
     * @brief Return the pool index of @p value, adding it if not present.
     * 
     * For numeric values (int32_t, uint32_t, float), a single slot is occupied.
     * For 64-bit values (int64_t, uint64_t, double), two consecutive slots are used.
     * String constants are interned through the fabric and marked with STRING_REF_MARKER.
     *
     * The slot index value starts from 0 and increments as new constants are added.
     * In the bytecode, a slot index below 256 can be encoded as a single byte directly, while larger
     * indices require an extended argument.
     * 
     * @return The pool index of the value.
     */
    ConstPool::IndexType find_or_add(const ValueType& value);

    std::size_t size() const {
        return numeric_to_index_.size() + string_to_index_.size();
    }

    /**
     * @brief Freeze the table into an immutable ConstPool for runtime use.
     * The returned pool maintains references to fabric-scoped strings.
     */
    std::shared_ptr<ConstPool> freeze() const {
        return std::make_shared<ConstPool>(store_);
    }

private:
    struct NumericHash {
        std::size_t operator()(const NumericType& value) const {
            return std::visit([&](auto&& arg) -> std::size_t {
                return std::hash<std::decay_t<decltype(arg)>>{}(arg) ^ (value.index() * 0x9e3779b9u);
            }, value);
        }
    };

    ConstPool store_;
    std::unordered_map<NumericType, ConstPool::IndexType, NumericHash> numeric_to_index_;
    std::unordered_map<std::string, ConstPool::IndexType> string_to_index_;
};
