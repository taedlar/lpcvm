// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "const_pool.hpp"
#include "fabric.hpp"
#include "lpc_types.hpp"
#include <spdlog/spdlog.h>

LpcStringRef ConstPool::fetch_string(IndexType index) const {
    // First check if this could be a valid string index at all
    if ((index & STRING_REF_MARKER) == 0) {
        // This looks like a numeric index or invalid index
        if (index < pool_.size()) {
            // Index is within numeric pool bounds, but it's not a string
            throw std::invalid_argument("ConstPool::fetch_string: index does not refer to a string constant");
        } else {
            // Index is completely out of range
            throw std::out_of_range("ConstPool::fetch_string: index out of range");
        }
    }

    uint32_t str_idx = index & STRING_REF_INDEX_MASK;
    if (str_idx >= string_refs_.size()) {
        throw std::out_of_range("ConstPool::fetch_string: string index out of range");
    }
    return string_refs_[str_idx];
}

ConstPool::IndexType ConstTable::find(const ValueType& value) const {
    if (auto* sv = std::get_if<std::string_view>(&value)) {
        std::string key(*sv);
        auto it = string_to_index_.find(key);
        if (it != string_to_index_.end())
            return it->second;
        return INVALID_INDEX;
    }

    return std::visit([&](auto&& arg) -> ConstPool::IndexType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string_view>) {
            return 0; // unreachable, handled above
        } else {
            NumericType key = arg;
            auto it = numeric_to_index_.find(key);
            if (it != numeric_to_index_.end())
                return it->second;
            return INVALID_INDEX;
        }
    }, value);
}

ConstPool::IndexType ConstTable::find_or_add(const ValueType& value) {
    if (auto* sv = std::get_if<std::string_view>(&value)) {
        std::string key(*sv);
        auto it = string_to_index_.find(key);
        if (it != string_to_index_.end())
            return it->second;

        if (store_.string_refs_.size() >= ConstPool::STRING_REF_INDEX_MASK) {
            throw std::overflow_error("ConstTable::find_or_add: too many string constants");
        }

        uint32_t str_index = static_cast<uint32_t>(store_.string_refs_.size());
        ConstPool::IndexType pool_index = str_index | ConstPool::STRING_REF_MARKER;
        auto interned = store_.fabric_.make_shared_string(*sv);
        store_.string_refs_.push_back(std::move(interned));
        try {
            string_to_index_.emplace(std::move(key), pool_index);
        } catch (...) {
            store_.string_refs_.pop_back();
            throw;
        }
        SPDLOG_DEBUG("Added string constant '{}' at index {} (interned)", *sv, pool_index);
        return pool_index;
    }

    return std::visit([&](auto&& arg) -> ConstPool::IndexType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string_view>) {
            return 0; // unreachable, handled above
        } else {
            NumericType key = arg;
            auto it = numeric_to_index_.find(key);
            if (it != numeric_to_index_.end())
                return it->second;

            constexpr size_t n_slots = sizeof(T) / sizeof(ConstPool::SlotType);
            ConstPool::IndexType index = static_cast<ConstPool::IndexType>(store_.pool_.size());
            store_.pool_.resize(store_.pool_.size() + n_slots);
            std::memcpy(&store_.pool_[index], &arg, sizeof(T));
            SPDLOG_DEBUG("Added numeric constant '{}' at index {}", arg, index);
            numeric_to_index_.emplace(key, index);
            return index;
        }
    }, value);
}
