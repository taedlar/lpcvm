// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_parser.hpp"
#include "vm_executor.hpp"
#include "object_class.hpp"
#include <iterator>

void ObjectClass::set_source_location(std::size_t bytecode_offset, ConstPool::IndexType source_file, std::size_t line_number) {
    if (!const_pool_)
        throw std::runtime_error("Source location requires a constant pool");
    source_locations_.insert_or_assign(bytecode_offset,
        SourceLocation{const_pool_->fetch_string(source_file), line_number});
}

std::optional<ObjectClass::SourceLocation> ObjectClass::lookup_source_location(const uint8_t* pc) const {
    if (!pc || bytecode_.empty() || source_locations_.empty())
        return std::nullopt;

    const auto address = reinterpret_cast<std::uintptr_t>(pc);
    const auto start = reinterpret_cast<std::uintptr_t>(bytecode_.data());
    if (address < start || address - start >= bytecode_.size())
        return std::nullopt;

    const auto next = source_locations_.upper_bound(address - start);
    if (next == source_locations_.begin())
        return std::nullopt;
    return std::prev(next)->second;
}

ObjectClass::ObjectClass(std::size_t num_vars, ConstTable* consts, SymbolTable* globals)
    : num_variables_(num_vars) {
    if (consts) {
        if (globals) {
            globals->emit_names(consts);
            // TODO: count variables only
            num_variables_ = globals->size();
        }
        const_pool_ = consts->make_intern();
    }
    virtual_table_ = std::make_shared<VirtualTable>(const_pool_);
}

void ObjectClass::inherit_from(std::shared_ptr<ObjectClass> parent) {
    if (parent) {
        for (auto& c: inherits_) {
            if (c == parent)
                return; // already inherited from this parent
        }
        inherits_.push_back(parent);
    }
}

bool ObjectClass::lookup_method(const std::string_view name, BytecodeView* entry) const {
    VirtualTable::Binding out_binding;
    if (virtual_table_ && virtual_table_->lookup(name, out_binding)) {
        if (entry) {
            BytecodeView found{
                bytecode_.data() + out_binding.bytecode_offset,
                out_binding.bytecode_length,
                this,
            };
            *entry = found;
        }
        return true;
    }
    return false;
}

const VirtualTable::Binding& ObjectClass::get_method(std::size_t index) const {
    if (!virtual_table_)
        throw std::runtime_error("VirtualTable not initialized");
    return (*virtual_table_)[index];
}
