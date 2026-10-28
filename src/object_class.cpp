// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_parser.hpp"
#include "vm_executor.hpp"
#include "object_class.hpp"

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
