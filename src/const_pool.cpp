// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "const_pool.hpp"

ConstPool::IndexType ConstTable::find_or_add(const ValueType& value) {
    auto it = value_to_index_.find(value);
    if (it != value_to_index_.end()) {
        // TODO: handle collisions for constant values
        return it->second;
    }
    IndexType index = static_cast<IndexType>(pool_.size());
    if (std::holds_alternative<std::string_view>(value)) {
        std::string_view str_view = std::get<std::string_view>(value);
        string_data_.insert(string_data_.end(), str_view.begin(), str_view.end());
        string_data_.push_back('\0');
        std::string_view str_view_in_data(string_data_.data() + string_data_.size() - str_view.size() - 1, str_view.size());
        pool_.push_back(str_view_in_data);
        value_to_index_[str_view_in_data] = index;
        return index;
    }
    pool_.push_back(value);
    value_to_index_[value] = index;
    return index;
}
