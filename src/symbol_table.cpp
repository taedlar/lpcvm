// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "symbol_table.hpp"

SymbolEntry* SymbolTable::lookup(const std::string& key) const {
    auto it = symbols_.find(key);
    if (it != symbols_.end()) {
        return it->second.get();
    }
    if (enclosing_scope_) {
        return enclosing_scope_->lookup(key);
    }
    return nullptr;
}

void SymbolTable::insert(const std::string& key, const std::shared_ptr<SymbolEntry>& entry) {
    symbols_[key] = entry;
}
