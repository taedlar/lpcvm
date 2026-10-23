// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <vector>
#include "vm_svalue.hpp"

class RuntimeState {
public:
    RuntimeState(std::size_t num_vars) {
        variables_.reserve(num_vars);
    }
    ~RuntimeState() = default;

private:
    std::vector<SValue> variables_;
};
