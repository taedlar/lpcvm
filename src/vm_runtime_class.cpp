// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_parser.hpp"
#include "vm_executor.hpp"
#include "vm_runtime_class.hpp"

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
            entry->start_ = bytecode_.data() + out_binding.bytecode_offset;
            entry->end_ = entry->start_ + out_binding.bytecode_length;
            entry->const_pool_ = const_pool_.get();
        }
        return true;
    }
    return false;
}
