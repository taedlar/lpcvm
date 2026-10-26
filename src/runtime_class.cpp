// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_parser.hpp"
#include "vm_executor.hpp"
#include "runtime_class.hpp"

RuntimeClass::RuntimeClass(ConstTable& consts, SymbolTable& symbols)
    : num_variables_(0) {
    const_pool_ = consts.make_intern();
    virtual_table_ = std::make_shared<VirtualTable>(const_pool_);
    symbols.emit_symbolic_bindings(virtual_table_);
    num_variables_ = symbols.size();
}

void RuntimeClass::inherit_from(std::shared_ptr<RuntimeClass> parent) {
    if (parent) {
        for (auto& c: inherits_) {
            if (c == parent)
                return; // already inherited from this parent
        }
        inherits_.push_back(parent);
    }
}

bool RuntimeClass::lookup_method(const std::string_view name, BytecodeView* entry) const {
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
