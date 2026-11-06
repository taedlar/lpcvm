// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include "lpc_types.hpp"
#include "fabric.hpp"
// #include "vm_sandbox.hpp"
#include "vm_svalue.hpp"

namespace lpcvm {

class DataStack {
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

    bool empty() const { return stack_.empty(); }

    std::size_t size() const { return stack_.size(); }

private:
    std::vector<SValue> stack_;

};

class ReturnStack {
public:
    struct Frame {
        const uint8_t* pc;      // program counter (address of next instruction)
        std::size_t fp;         // frame pointer for the current frame (position of first argument in data stack)
        std::size_t vbp;        // variable base pointer (position of global variable #0 in self_)
        const uint8_t* start;   // start of the bytecode segment
        const uint8_t* end;     // end of the bytecode segment
    };

    void push(const uint8_t* pc, std::size_t fp, std::size_t vbp, const uint8_t* start, const uint8_t* end) {
        stack_.push_back({pc, fp, vbp, start, end});
    }

    void pop() {
        if (stack_.empty()) {
            throw std::runtime_error("Return stack underflow");
        }
        stack_.pop_back();
    }

    Frame& top() {
        if (stack_.empty()) {
            throw std::runtime_error("Return stack is empty");
        }
        return stack_.back();
    }

    bool empty() const { return stack_.empty(); }

    std::size_t size() const { return stack_.size(); }

private:
    std::vector<Frame> stack_;
};

/**
 * @brief Represents a RAII execution context for a function or method in the
 * virtual machine.
 *
 * This structure holds the bytecode segment, the associated object class,
 * and the current object state. It is used to track the execution of a
 * function or method, including its start and end positions in the bytecode.
 *
 * When constructed with a LpcObjectRef, the Execution struct holds a strong
 * reference to the object to ensure it remains alive during execution (RAII).
 */
struct Execution {
    using FabricType = ManagedRuntime;

    Execution() = default;

    Execution(FabricType* sb)
        : Execution() {
        fabric_ = sb;
    }

    Execution(const uint8_t* bytecode, std::size_t size, const ObjectClass* prog = nullptr)
        : start_(bytecode), end_(bytecode + size), prog_(prog), self_(nullptr), var0_(0), fabric_(nullptr) {
        // construct with binding
    }

    Execution(const uint8_t* bytecode, std::size_t size, std::size_t offset, LpcObjectRef obj)
        : start_(bytecode), end_(bytecode + size), var0_(offset), obj_ref_(obj) {
        if (obj_ref_) {
            prog_ = obj_ref_->program().get();
            self_ = obj_ref_->state().get();
            fabric_ = obj_ref_->fabric();
        }
        else {
            prog_ = nullptr;
            self_ = nullptr;
            fabric_ = nullptr;
        }
    }

    std::size_t length() const { return (start_ && end_) ? (end_ - start_) : 0; }

    Execution& binding(const uint8_t* bytecode, std::size_t size, const ObjectClass* prog = nullptr) {
        start_ = bytecode;
        end_ = bytecode + size;
        prog_ = prog;
        return *this;
    }

    Execution& as_with(ObjectState* self, std::size_t offset = 0) {
        self_ = self;
        var0_ = offset;
        return *this;
    }

    int execute(ReturnStack& control, DataStack& stack, std::size_t num_args = 0);

    const uint8_t* start_;
    const uint8_t* end_;
    const ObjectClass* prog_; // bytecode site to get F_CALL entry
    ObjectState* self_;
    std::size_t var0_; // position of global variable #0 in self_ (inheritance offset)
    FabricType* fabric_;
    LpcObjectRef obj_ref_;  // holds a strong reference when constructed with LpcObjectRef
};

} // namespace lpcvm
