// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <limits>
#include <memory>
#include "lpc_types.hpp"
#include "fabric.hpp"
#include "object_state.hpp"
#include "vm_svalue.hpp"

namespace lpcvm {

class DataStack {
public:
    DataStack() = default;
    DataStack(DataStack&& other) = delete; // prevent move construction
    DataStack(const DataStack& other) = delete; // prevent copy construction

    const SValue& operator[](std::size_t index) const {
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
        if (std::holds_alternative<TemporaryString>(value)) {
            throw std::runtime_error("Cannot push a TemporaryString directly");
        }
        SValue copy = value;
        if (stack_.size() + 1 > stack_.capacity()) {
            stack_.reserve(stack_.size() + CHUNK_SIZE);
        }
        stack_.push_back(std::move(copy));
    }

    void push_temp_string(std::string_view str) {
        constexpr auto max = std::numeric_limits<uint32_t>::max();
        if (str.size() > max || cat_buffer_.size() > max - str.size()) {
            throw std::length_error("Temporary string buffer exceeds its representable size");
        }
        TemporaryString tmp;
        tmp.offset = cat_buffer_.size();
        tmp.length = str.size();
        cat_buffer_.append(str);
        try {
            stack_.push_back(tmp);
        } catch (...) {
            cat_buffer_.resize(tmp.offset);
            throw;
        }
    }

    // The returned view is invalidated by mutations of the temporary string buffer.
    std::string_view fetch_temp_string(std::size_t sp) const {
        if (sp >= stack_.size()) {
            throw std::runtime_error("Evaluation stack index out of bounds");
        }
        const SValue& value = stack_[sp];
        if (!std::holds_alternative<TemporaryString>(value)) {
            throw std::runtime_error("Expected a temporary string at the given stack position");
        }
        TemporaryString tmp = std::get<TemporaryString>(value);
        if (tmp.length == 0)
            return {};
        return std::string_view(cat_buffer_.data() + tmp.offset, tmp.length);
    }

    std::string_view top_string() const {
        const auto& value = top();
        if (std::holds_alternative<TemporaryString>(value))
            return fetch_temp_string(stack_.size() - 1);
        if (const auto* string = std::get_if<LpcStringRef>(&value); string && *string)
            return (*string)->view();
        throw std::runtime_error("Expected a string operand");
    }

    void concat_top_strings() {
        if (stack_.size() < 2)
            throw std::runtime_error("Evaluation stack underflow");
        const auto string_view_at = [this](std::size_t pos) -> std::string_view {
            const auto& value = (*this)[pos];
            if (std::holds_alternative<TemporaryString>(value))
                return fetch_temp_string(pos);
            if (const auto* str = std::get_if<LpcStringRef>(&value))
                return (*str)->view();
            throw std::runtime_error("Unsupported binary operand types");
        };
        const auto left = string_view_at(stack_.size() - 2);
        const auto right = string_view_at(stack_.size() - 1);
        constexpr auto max = std::numeric_limits<uint32_t>::max();
        if (left.size() > max || right.size() > max - left.size())
            throw std::length_error("Temporary string buffer exceeds its representable size");
        std::string combined;
        combined.reserve(left.size() + right.size());
        combined.append(left);
        combined.append(right);
        push_temp_string(combined);
        pop_i(stack_.size() - 3, 2);
    }

    void push_copy(std::size_t source) {
        const SValue& value = (*this)[source];
        if (std::holds_alternative<TemporaryString>(value)) {
            std::string copy(fetch_temp_string(source));
            push_temp_string(copy);
        } else {
            push(value);
        }
    }

    void assign_copy(std::size_t destination, std::size_t source) {
        (*this)[destination];
        (*this)[source];
        if (destination == source)
            return;
        push_copy(source);
        release(destination);
        stack_[destination] = std::move(stack_.back());
        stack_.pop_back();
    }

    void assign(std::size_t destination, const SValue& value) {
        (*this)[destination];
        if (std::holds_alternative<TemporaryString>(value)) {
            throw std::runtime_error("Cannot assign a TemporaryString directly");
        }
        SValue copy = value;
        release(destination);
        stack_[destination] = std::move(copy);
    }

    void pop_n(std::size_t n) {
        if (n > stack_.size())
            throw std::runtime_error("Evaluation stack underflow");
        while (n--)
            pop();
    }

    void pop_i(std::size_t pos, std::size_t n) {
        if (pos > stack_.size() || n > stack_.size() - pos) {
            throw std::runtime_error("Evaluation stack underflow");
        }
        for (std::size_t i = pos; i < pos + n; ++i)
            release(i);
        stack_.erase(stack_.begin() + pos, stack_.begin() + pos + n);
    }

    SValue pop() {
        if (stack_.empty())
            throw std::runtime_error("Evaluation stack underflow");
        SValue value = stack_.back();
        if (std::holds_alternative<TemporaryString>(value)) {
            value = std::monostate{};
        }
        release(stack_.size() - 1);
        stack_.pop_back();
        return value;
    }

    const SValue& top() const {
        if (stack_.empty()) {
            throw std::runtime_error("Evaluation stack is empty");
        }
        return stack_.back();
    }

    bool empty() const { return stack_.empty(); }

    std::size_t size() const { return stack_.size(); }

private:
    void release(std::size_t pos) {
        auto* tmp = std::get_if<TemporaryString>(&stack_[pos]);
        if (!tmp)
            return;
        const std::size_t offset = tmp->offset;
        const std::size_t length = tmp->length;
        stack_[pos] = std::monostate{};
        if (length == 0)
            return;
        if (offset + length == cat_buffer_.size()) {
            cat_buffer_.resize(offset);
            return;
        }
        cat_buffer_.erase(offset, length);
        for (auto& value : stack_) {
            if (auto* other = std::get_if<TemporaryString>(&value)) {
                if (other->length != 0 && other->offset >= offset + length)
                    other->offset -= length;
            }
        }
    }

    std::vector<SValue> stack_;
    std::string cat_buffer_; // buffer for all pushed strings, optimized for string concatenation
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

    Execution(std::shared_ptr<FabricType> sb)
        : Execution() {
        fabric_ = std::move(sb);
    }

    Execution(const uint8_t* bytecode, std::size_t size, const ObjectClass* prog = nullptr)
        : start_(bytecode), end_(bytecode + size), prog_(prog), self_(nullptr), var0_(0), fabric_(nullptr) {
    }

    Execution(const uint8_t* bytecode, std::size_t size, std::size_t offset, LpcObjectRef obj)
        : start_(bytecode), end_(bytecode + size), var0_(offset), obj_ref_(obj) {
        if (obj_ref_) {
            prog_ = obj_ref_->program().get();
            self_ = obj_ref_->state();
            fabric_ = obj_ref_->retain_fabric();
        }
        else {
            prog_ = nullptr;
            self_ = nullptr;
            fabric_ = nullptr;
        }
    }

    std::size_t length() const { return (start_ && end_) ? (end_ - start_) : 0; }

    Execution& binding(std::shared_ptr<FabricType> sb, const uint8_t* bytecode, std::size_t size, const ObjectClass* prog = nullptr) {
        fabric_ = std::move(sb);
        start_ = bytecode;
        end_ = bytecode + size;
        prog_ = prog;
        return *this;
    }

    Execution& as_with(ObjectState* self, std::size_t offset = 0);

    int execute(ReturnStack& control, DataStack& stack, std::size_t num_args = 0);

    const uint8_t* start_ = nullptr;
    const uint8_t* end_ = nullptr;
    const ObjectClass* prog_ = nullptr; // bytecode site to get F_CALL entry
    ObjectState* self_ = nullptr;
    std::size_t var0_ = 0; // position of global variable #0 in self_ (inheritance offset)
    std::shared_ptr<FabricType> fabric_; // fabric associated with the execution context
    LpcObjectRef obj_ref_;  // holds a strong reference when constructed with LpcObjectRef
};

} // namespace lpcvm
