// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include "const_pool.hpp"
#include "runtime_class.hpp"
#include "runtime_state.hpp"
#include "vm_svalue.hpp"

class OperandStack {
private:
    std::vector<SValue> stack_;

public:
    SValue& operator[](std::size_t index) {
        if (index >= stack_.size()) {
            throw std::runtime_error("Evaluation stack index out of bounds");
        }
        return stack_[index];
    }

    const std::size_t CHUNK_SIZE = 16;

    void push_n(std::size_t n) {
        if (stack_.size() + n > stack_.capacity()) {
            stack_.reserve(stack_.size() + CHUNK_SIZE);
        }
        stack_.resize(stack_.size() + n);
    }

    void push(const SValue& value) {
        if (stack_.size() + 1 > stack_.capacity()) {
            stack_.reserve(stack_.size() + CHUNK_SIZE);
        }
        stack_.push_back(value);
    }

    void pop_n(std::size_t n) {
        if (n > stack_.size()) {
            throw std::runtime_error("Evaluation stack underflow");
        }
        stack_.resize(stack_.size() - n);
    }

    void pop_i(std::size_t pos, int n) {
        if (n <= 0)
            return;
        if (pos + n > stack_.size()) {
            throw std::runtime_error("Evaluation stack underflow");
        }
        stack_.erase(stack_.begin() + pos, stack_.begin() + pos + n);
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

struct BytecodeView {
public:
    BytecodeView()
        : start_(nullptr), end_(nullptr), prog_(nullptr), this_obj_(nullptr) {}
    BytecodeView(const uint8_t* bytecode, std::size_t size, const RuntimeClass* prog = nullptr, RuntimeState* obj = nullptr)
        : start_(bytecode), end_(bytecode + size), prog_(prog), this_obj_(obj) {}
    BytecodeView(const BytecodeView&) = default;
    BytecodeView& operator=(const BytecodeView&) = default;

    std::size_t length() const { return start_ ? (end_ - start_) : 0; }
    int execute(OperandStack& stack, int num_args = 0);

private:
    const uint8_t* start_;
    const uint8_t* end_;
    const RuntimeClass* prog_;
    RuntimeState* this_obj_;
};
