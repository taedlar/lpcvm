// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <vector>
#include "vm_svalue.hpp"

class ObjectState {
public:
    ObjectState(std::size_t num_vars) {
        variables_.resize(num_vars);
    }
    ~ObjectState() = default;

    SValue& operator[](std::size_t index) {
        if (index >= variables_.size()) {
            throw std::out_of_range("Variable index out of range");
        }
        return variables_[index];
    }

    const SValue& operator[](std::size_t index) const {
        if (index >= variables_.size()) {
            throw std::out_of_range("Variable index out of range");
        }
        return variables_[index];
    }

private:
    std::vector<SValue> variables_;
};
