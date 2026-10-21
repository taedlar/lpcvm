// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include "const_pool.hpp"
#include "vm_svalue.hpp"

class EvaluationStack {
private:
    std::vector<SValue> stack_;

public:
    void push(const SValue& value) {
        stack_.push_back(value);
    }

    SValue pop() {
        if (stack_.empty()) {
            throw std::runtime_error("Evaluation stack underflow");
        }
        SValue value = stack_.back();
        stack_.pop_back();
        return value;
    }

    SValue& top() {
        if (stack_.empty()) {
            throw std::runtime_error("Evaluation stack is empty");
        }
        return stack_.back();
    }

    bool empty() const {
        return stack_.empty();
    }

    std::size_t size() const {
        return stack_.size();
    }
};

class BytecodeView {
public:
    BytecodeView(const uint8_t* bytecode, std::size_t size, const ConstPool* const_pool)
        : start_(bytecode), end_(bytecode + size), const_pool_(const_pool) {}

    void execute(EvaluationStack& stack);

private:
    const uint8_t* start_;
    const uint8_t* end_;
    const ConstPool* const_pool_;
};
