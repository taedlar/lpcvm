// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>

/** 
 * @brief A pool of constant values for efficient storage and lookup.
 */
class ConstPool {
public:
    ConstPool() = default;
    ~ConstPool() {};

    using ValueType = std::variant<long, float, double, std::string>;
    using IndexType = uint32_t; // runtime index (this is the max width, extended from 8-bit via F_EXTENDED_ARG in bytecode)

    const ValueType& operator[](IndexType index) const {
        return pool_[index];
    }

protected:
    std::vector<ValueType> pool_; // runtime random access at O(1)
};

/**
 * @brief A compile-time constant table that extends ConstPool with efficient
 * lookup and addition of constants.
 */
class ConstTable: public ConstPool {
public:
    ConstTable() = default;
    ~ConstTable() {};

    IndexType find_or_add(const ValueType& value);

private:
    struct ValueHash {
        std::size_t operator() (const ValueType& value) const {
            return std::visit([](auto&& arg) -> std::size_t {
                return std::hash<std::decay_t<decltype(arg)>>{}(arg);
            }, value);
        }
    };
    std::unordered_map<ValueType, IndexType, ValueHash> value_to_index_; // compile-time lookup at O(1)
};
