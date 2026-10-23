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

class BytecodeView;

class RuntimeClass {
public:
    RuntimeClass()
        : num_variables_(0) {};
    ~RuntimeClass() = default;

    /**
     * \brief Sets the number of instance variables for this runtime class.
     *
     * \param num The number of instance variables (excluding inherited variables).
     */
    void set_num_variables(std::size_t num) { num_variables_ = num; }
    std::size_t num_variables() const { return num_variables_; }
    std::size_t num_variables_inheriting() const {
        std::size_t total = num_variables_;
        for (const auto& parent : inherits_) {
            total += parent->num_variables_inheriting();
        }
        return total;
    }

    const ConstPool* const_pool() const { return const_pool_.get(); }

    bool lookup_method (const std::string_view name, BytecodeView* entry) const;

    void inherit_from(std::shared_ptr<RuntimeClass> parent);

private:
    friend class LpcCompiler;
    std::vector<uint8_t> bytecode_;
    std::vector<std::shared_ptr<RuntimeClass>> inherits_;
    std::shared_ptr<ConstPool> const_pool_;
    std::shared_ptr<VirtualTable> virtual_table_;
    std::size_t num_variables_; // (not including inherited variables)
};
