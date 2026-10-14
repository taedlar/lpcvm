// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "const_pool.hpp"

ConstPool::IndexType ConstTable::find_or_add(const ValueType& value) {
    if (auto* sv = std::get_if<std::string_view>(&value)) {
        std::string key(*sv);
        auto it = string_to_index_.find(key);
        if (it != string_to_index_.end())
            return it->second;

        IndexType index = static_cast<IndexType>(pool_.size());
        pool_.push_back(static_cast<SlotType>(string_data_.size())); // offset
        pool_.push_back(static_cast<SlotType>(sv->size()));          // length
        string_data_.append(*sv);
        string_data_.push_back('\0');
        SPDLOG_DEBUG("Added string constant '{}' at index {}", *sv, index);
        string_to_index_.emplace(std::move(key), index);
        return index;
    }

    return std::visit([&](auto&& arg) -> IndexType {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string_view>) {
            return 0; // unreachable, handled above
        } else {
            NumericType key = arg;
            auto it = numeric_to_index_.find(key);
            if (it != numeric_to_index_.end())
                return it->second;

            constexpr size_t n_slots = sizeof(T) / sizeof(SlotType);
            IndexType index = static_cast<IndexType>(pool_.size());
            pool_.resize(pool_.size() + n_slots);
            std::memcpy(&pool_[index], &arg, sizeof(T));
            SPDLOG_DEBUG("Added numeric constant '{}' at index {}", arg, index);
            numeric_to_index_.emplace(key, index);
            return index;
        }
    }, value);
}
