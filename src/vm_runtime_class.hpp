// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "const_pool.hpp"
#include "virtual_table.hpp"

class RuntimeClass {
public:
    RuntimeClass() = default;
    ~RuntimeClass() = default;

    using MemberIndexType = ConstPool::IndexType; // index to member name in the constant pool

    void inherit_from(std::shared_ptr<RuntimeClass> parent);

    /// @brief Returns the total number of instance variables for this runtime class,
    /// including those inherited from parent classes.
    /// @return The total number of instance variables.
    std::size_t num_instance_variables() const {
        std::size_t total = num_variables_;
        for (const auto& parent : inherits_) {
            total += parent->num_instance_variables();
        }
        return total;
    }

    bool lookup_method (const std::string_view name, VirtualTable::Binding& out_binding) const {
        return virtual_table_ ? virtual_table_->lookup(name, out_binding) : false;
    }

private:
    std::vector<uint8_t> bytecode_;
    std::vector<std::shared_ptr<RuntimeClass>> inherits_;
    std::unique_ptr<ConstPool> const_pool_;
    std::unique_ptr<VirtualTable> virtual_table_;
    MemberIndexType num_variables_; // (not including inherited variables)
};
