// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "const_pool.hpp"

ConstPool::IndexType ConstTable::find_or_add(const ValueType& value) {
    auto it = value_to_index_.find(value);
    if (it != value_to_index_.end()) {
        return it->second;
    }
    IndexType index = static_cast<IndexType>(pool_.size());
    pool_.push_back(value);
    value_to_index_[value] = index;
    return index;
}
