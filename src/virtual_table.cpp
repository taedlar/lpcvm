// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
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
        out_binding = it->second;
        return true;
    }
    return false;
}

void VirtualTable::emplace (ConstPool::IndexType name_index, const Binding& binding) {
    auto name = name_pool_->fetch_string(name_index);
    auto it = std::lower_bound(
        bindings_.begin(),
        bindings_.end(),
        name,
        [](const auto& lhs, const auto& rhs) { return lhs.first < rhs; }
    );
    if (it == bindings_.end() || it->first != name) {
        bindings_.insert(it, {name, binding});
    } else {
        it->second = binding;
    }
}
