// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "virtual_table.hpp"
#include <algorithm>

bool VirtualTable::lookup (const std::string_view name, Binding& out_binding) const {
    auto it = std::lower_bound(
        bindings_.begin(),
        bindings_.end(),
        name,
        [](const auto& lhs, const auto& rhs) { return lhs.first < rhs; }
    );
    if (it != bindings_.end() && it->first == name) {
        out_binding = static_bindings_.at(it->second);
        return true;
    }
    return false;
}

bool VirtualTable::bind (std::size_t index, const Binding& binding) {
    if (index >= static_bindings_.size()) {
        SPDLOG_ERROR("VirtualTable: static binding index out of range: {}", index);
        return false;
    }
    static_bindings_[index] = binding;
    return true;
}

void VirtualTable::emplace (ConstPool::IndexType name_index, std::size_t index) {
    auto name = name_pool_->fetch_string(name_index)->view();
    auto it = std::lower_bound(
        bindings_.begin(),
        bindings_.end(),
        name,
        [](const auto& lhs, const auto& rhs) { return lhs.first < rhs; }
    );
    if (it == bindings_.end() || it->first != name) {
        bindings_.insert(it, {name, index});
    } else {
        it->second = index;
    }
}
