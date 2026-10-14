// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "lpc_parser.hpp"
#include "vm_runtime_class.hpp"

std::unordered_map<std::string, std::shared_ptr<RuntimeClass>> RuntimeClass::registry_;

void RuntimeClass::register_class(const std::string& path, std::shared_ptr<RuntimeClass> runtime_class) {
    registry_[path] = runtime_class;
}

std::shared_ptr<RuntimeClass> RuntimeClass::get_class(const std::string& path) {
    auto it = registry_.find(path);
    if (it != registry_.end()) {
        return it->second;
    }
    return nullptr;
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
