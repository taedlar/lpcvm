// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "native_module.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

#include "vm_executor.hpp"

namespace {

static SValue container_value(const lpcvm::DataStack& stack, std::size_t slot,
                             const std::shared_ptr<ManagedRuntime>& fabric) {
    const auto& value = stack[slot];
    if (std::holds_alternative<TemporaryString>(value) || std::holds_alternative<LpcBufferRef>(value)) {
        if (!fabric)
            throw std::runtime_error("Unable to store a string without a fabric");
        return fabric->make_shared_string(stack.string_at(slot));
    }
    return value;
}

static int adjust_call_arguments(const SymbolSignature& signature, lpcvm::DataStack& stack, std::size_t fp,
                                 int num_args, const std::shared_ptr<ManagedRuntime>& fabric) {
    if (num_args < 0)
        throw std::runtime_error("Invalid argument count");

    const std::size_t actual = static_cast<std::size_t>(num_args);
    const std::size_t formal = signature.b.num_formal_params;
    if (formal > std::numeric_limits<int>::max())
        throw std::runtime_error("Formal parameter count exceeds limits");

    if (signature.b.is_vararg) {
        if (formal == 0)
            throw std::runtime_error("Vararg entry requires a final formal parameter");
        const std::size_t fixed = formal - 1;
        const bool has_keywords = actual > 0 && std::holds_alternative<LpcMappingRef>(stack.top());
        const std::size_t positional = actual - (has_keywords ? 1 : 0);
        if (positional > UINT32_MAX)
            throw std::length_error("Too many positional arguments");

        LpcMappingRef mapping;
        if (has_keywords) {
            const auto* supplied = std::get_if<LpcMappingRef>(&stack.top());
            if (!supplied || !*supplied)
                throw std::runtime_error("Expected a keyword argument mapping");
            mapping = *supplied;
        } else {
            if (!fabric)
                throw std::runtime_error("Vararg mapping creation requires a fabric");
            mapping = fabric->make_container<LpcMappingRef>(positional > fixed ? positional - fixed : 0);
        }

        for (std::size_t i = fixed; i < positional; ++i) {
            // Match integer literals: zero is signed, positive positions are unsigned.
            const SValue key = i == 0 ? SValue(int32_t{0}) : SValue(static_cast<uint32_t>(i));
            mapping->find_or_insert({key}).value = container_value(stack, fp + i, fabric);
        }

        if (has_keywords)
            stack.pop();
        if (positional > fixed)
            stack.pop_n(positional - fixed);
        else if (positional < fixed)
            stack.push_n(fixed - positional);
        stack.push(mapping);
        return static_cast<int>(formal);
    }

    if (actual < formal)
        stack.push_n(formal - actual);
    else if (actual > formal)
        stack.pop_n(actual - formal);
    return static_cast<int>(formal);
}

} // namespace

void NativeModule::add_method(std::string_view name, SymbolSignature signature, NativeMethod method) {
    if (!method)
        throw std::invalid_argument("Native method must not be null");
    if (finalized_)
        throw std::logic_error("NativeModule method table already finalized");
    auto fabric = retain_fabric();
    if (!fabric)
        throw std::runtime_error("NativeModule requires a live fabric to intern method names");

    MethodEntry entry;
    entry.name = fabric->make_shared_string(name);
    entry.signature = signature;
    entry.method = method;
    methods_.push_back(std::move(entry));
}

void NativeModule::finalize_methods() {
    const auto by_identity = [](const MethodEntry& lhs, const MethodEntry& rhs) {
        return std::less<const LpcString*>{}(lhs.identity(), rhs.identity());
    };

    std::sort(methods_.begin(), methods_.end(), by_identity);

    const auto duplicates = std::adjacent_find(
        methods_.begin(), methods_.end(), [](const MethodEntry& lhs, const MethodEntry& rhs) {
            return lhs.identity() == rhs.identity();
        });
    if (duplicates != methods_.end())
        throw std::invalid_argument("NativeModule methods must have unique names");

    finalized_ = true;
}

int NativeModule::call_method(void* func_identity, lpcvm::DataStack* data_stack,
                              lpcvm::ReturnStack* return_stack, int num_args) {
    if (!finalized_ || !func_identity)
        return -1;
    if (!data_stack || !return_stack || num_args < 0 ||
        static_cast<std::size_t>(num_args) > data_stack->size())
        return -1;

    const auto* identity = static_cast<const LpcString*>(func_identity);
    const auto it = std::lower_bound(
        methods_.begin(), methods_.end(), identity, [](const MethodEntry& entry, const LpcString* key) {
            return std::less<const LpcString*>{}(entry.identity(), key);
        });
    if (it == methods_.end() || it->identity() != identity)
        return -1;

    const std::size_t fp = data_stack->size() - static_cast<std::size_t>(num_args);
    const auto fabric = retain_fabric();
    const int adjusted = adjust_call_arguments(it->signature, *data_stack, fp, num_args, fabric);
    return it->method(this, data_stack, return_stack, adjusted);
}

