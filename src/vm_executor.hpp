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
    BytecodeView() : start_(nullptr), end_(nullptr), const_pool_(nullptr), globals_(nullptr) {}
    BytecodeView(const uint8_t* bytecode, std::size_t size, const ConstPool* const_pool = nullptr, SValue* globals = nullptr)
        : start_(bytecode), end_(bytecode + size), const_pool_(const_pool), globals_(globals) {}
    BytecodeView(const BytecodeView&) = default;
    BytecodeView& operator=(const BytecodeView&) = default;

    std::size_t length() const { return start_ ? (end_ - start_) : 0; }
    int execute(EvaluationStack& stack, int num_args = 0);

private:
    const uint8_t* start_;
    const uint8_t* end_;
    const ConstPool* const_pool_;
    SValue* globals_;
};
