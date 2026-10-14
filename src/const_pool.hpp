// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

/** 
 * @brief A flattened array of constant values, used by the VM runtime to
 * retrieve constants.
 *
 * The minimum allocation unit is SlotType. The pool stores all constants
 * in terms of these slots, ensuring alignment and relocatability.
 *
 * Numeric values occupy sizeof(T)/sizeof(SlotType) consecutive slots.
 * Strings occupy two slots {offset, length} referring to string_data_, so
 * the pool stays relocatable.
 *
 * Type of value is not stored explicitly; it is encoded with opcodes that
 * loads the constant from the pool.
 */
class ConstPool {
public:
    ConstPool() = default;
    ~ConstPool() = default;

    using SlotType = uint32_t; // flattened unit of storage
    using IndexType = uint32_t; // runtime index (this is the max width, extended from 8-bit via F_EXTENDED_ARG in bytecode)

    /** @brief Read a numeric constant at slot @p index (alignment-safe). */
    template<typename T>
    T fetch(IndexType index) const {
        static_assert(std::is_arithmetic_v<T>, "use as_string() for strings");
        static_assert(sizeof(T) % sizeof(SlotType) == 0);
        T value;
        std::memcpy(&value, &pool_[index], sizeof(T));
        return value;
    }

    /** @brief Read a string constant at slot @p index; the view is valid for the pool's lifetime. */
    std::string_view fetch_string(IndexType index) const {
        return std::string_view(string_data_.data() + pool_[index], pool_[index + 1]);
    }

    size_t slot_count() const {
        return pool_.size();
    }

protected:
    std::vector<SlotType> pool_; // runtime random access at O(1)
    std::string string_data_;    // NUL-terminated string literals, referenced by offset from pool_
};

/**
 * @brief A compile-time constant table that extends ConstPool with efficient
 * lookup and addition of constants.
 */
class ConstTable: public ConstPool {
public:
    ConstTable() = default;
    ~ConstTable() = default;

    using ValueType = std::variant<int32_t, uint32_t, int64_t, uint64_t, float, double, std::string_view>;

    /** @brief Return the slot index of @p value, adding it if not present
     * 
     * If the value is already present, its existing slot index is returned.
     * An int32_t, uint32_t, float occupies a single slot, while int64_t, uint64_t, and
     * double occupy two consecutive slots.
     * String constants occupy two consecutive slots as {offset, length}.
     *
     * The slot index value starts from 0 and increments as new constants are added.
     * In the bytecode, a slot index below 256 can be encoded as a single byte directly, while larger
     * indices require an extended argument.
     * 
     * @return The slot index of the value.
     */
    IndexType find_or_add(const ValueType& value);

    std::size_t size() const {
        return numeric_to_index_.size() + string_to_index_.size();
    }

private:
    using NumericType = std::variant<int32_t, uint32_t, int64_t, uint64_t, float, double>;

    struct NumericHash {
        std::size_t operator()(const NumericType& value) const {
            return std::visit([&](auto&& arg) -> std::size_t {
                return std::hash<std::decay_t<decltype(arg)>>{}(arg) ^ (value.index() * 0x9e3779b9u);
            }, value);
        }
    };

    /**
     * @brief Mapping from numeric constants to their slot indices.
     */
    std::unordered_map<NumericType, IndexType, NumericHash> numeric_to_index_;

    /**
     * @brief Mapping from string constants to their slot indices.
     * @note The string is stored in the string_data_ member, and the pool_
     * contains the offset and length. Using map-owned std::string is required
     * because the ConstTable allows addition of new strings, which may reallocate
     * the string_data_ buffer, invalidating any existing string_view references.
     */
    std::unordered_map<std::string, IndexType> string_to_index_;
};
