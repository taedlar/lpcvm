// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "symbol_table.hpp"

std::shared_ptr<SymbolTable::Entry> SymbolTable::lookup(std::string_view key) const {
    auto it = symbols_.find(std::string(key));
    if (it != symbols_.end()) {
        return it->second;
    }
    if (enclosing_scope_) {
        return enclosing_scope_->lookup(key);
    }
    return nullptr;
}

void SymbolTable::emplace(std::string_view key, const std::shared_ptr<SymbolTable::Entry>& entry) {
    symbols_[std::string(key)] = entry;
}
