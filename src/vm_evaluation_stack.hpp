// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include "vm_svalue.hpp"

class EvaluationStack {
private:
    std::vector<SValue> stack_;
    std::vector<uint8_t> arena_;

public:
    void push(const SValue &value) {
        stack_.push_back(value);
    }

    SValue pop() {
        if (stack_.empty()) {
            return SValue{};
        }
        SValue value = stack_.back();
        stack_.pop_back();
        return value;
    }

    SValue top() const {
        if (stack_.empty()) {
            return SValue{};
        }
        return stack_.back();
    }

    bool empty() const {
        return stack_.empty();
    }
};
