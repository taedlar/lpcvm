// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "symbol_table.hpp"

void SymbolTable::Entry::push_token(SymbolTable& owner, TokenIndex token) {
    if (!value.has_value()) {
        value = token;
        return;
    }
    auto* next = &std::get<TokenIndex>(*value);
    while (*next != invalid_token)
        next = &owner.tokens_.at(static_cast<std::size_t>(*next)).next;
    *next = token;
}

void SymbolTable::Entry::prepend_token(SymbolTable& owner, TokenIndex token) {
    if (!value.has_value()) {
        value = token;
        return;
    }
    auto* next = &std::get<TokenIndex>(*value);
    auto old_head = *next;
    *next = token;
    owner.tokens_.at(static_cast<std::size_t>(token)).next = old_head;
}

std::shared_ptr<SymbolTable::Entry> SymbolTable::lookup(const std::string_view key, const SymbolTable** owner) const {
    auto it = symbols_.find(std::string(key));
    if (it != symbols_.end()) {
        if (owner) *owner = this;
        return it->second;
    }
    if (enclosing_scope_) {
        return enclosing_scope_->lookup(key, owner);
    }
    if (owner) *owner = nullptr;
    return nullptr;
}

void SymbolTable::emplace(const std::string_view key, std::shared_ptr<SymbolTable::Entry> entry) {
    symbols_[std::string(key)] = std::move(entry);
}

void SymbolTable::emit_names(ConstTable* consts) const {
    for (const auto& [key, entry] : symbols_) {
        auto sig = entry->signature;
        if (!sig.b.is_function)
            continue; // only process function entries
        if (!sig.b.is_const) {
            if (sig.b.referenced)
                throw std::runtime_error("Function referenced but not defined: " + key);
            continue; // unreferenced function forward declarations
        }
        consts->find_or_add(key); // intern the function name in the constant pool
    }
}
