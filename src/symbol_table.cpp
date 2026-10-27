// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "symbol_table.hpp"

std::shared_ptr<SymbolTable::Entry> SymbolTable::lookup(const std::string_view key) const {
    auto it = symbols_.find(std::string(key));
    if (it != symbols_.end()) {
        return it->second;
    }
    if (enclosing_scope_) {
        return enclosing_scope_->lookup(key);
    }
    return nullptr;
}

void SymbolTable::emplace(const std::string_view key, std::shared_ptr<SymbolTable::Entry> entry) {
    symbols_[std::string(key)] = std::move(entry);
}

void SymbolTable::emit_symbolic_bindings(std::shared_ptr<VirtualTable> vt) const {
    for (const auto& [key, entry] : symbols_) {
        //vt->bind(key, entry);
    }
}
