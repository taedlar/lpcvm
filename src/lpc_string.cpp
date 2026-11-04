// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "lpc_types.hpp"
#include "managed_runtime.hpp"
#include <cstring>
#include <new>

namespace {
constexpr auto string_overhead = sizeof(LpcStringRef::ControlBlock) + sizeof(LpcString) + 1;
}

LpcStringRef lpcvm::make_shared_string(ManagedRuntime& runtime, std::string_view value) {
    return runtime.make_shared_string(value);
}

LpcStringRef ManagedRuntime::make_shared_string(std::string_view value) {
    auto found = strings_.find(value);
    if (found != strings_.end())
        return found->second;

    auto result = allocate_shared_string(value);
    // The table's strong reference keeps the borrowed key alive.
    strings_.emplace(result->view(), result);
    return result;
}

LpcStringRef ManagedRuntime::allocate_shared_string(std::string_view value) {
    using ControlBlock = LpcStringRef::ControlBlock;
    static_assert(alignof(LpcString) <= alignof(ControlBlock)
                  || sizeof(ControlBlock) % alignof(LpcString) == 0,
                  "string must be aligned after the control block");
    if (value.size() > std::numeric_limits<std::size_t>::max() - string_overhead)
        throw std::length_error("LpcString is too large");

    // One allocation: [reference counts][LpcString][bytes][NUL].
    void* memory = ::operator new(string_overhead + value.size());
    auto* control = new (memory) ControlBlock{1, 0};
    auto* object = new (static_cast<void*>(control + 1)) LpcString(value.size());
    auto* bytes = reinterpret_cast<char*>(object + 1);
    if (!value.empty())
        std::memcpy(bytes, value.data(), value.size());
    bytes[value.size()] = '\0';
    // Adopt before inserting so allocation failure in the table releases it.
    LpcStringRef result(object, LpcStringRef::AdoptRetainedReference{});
    return result;
}

std::size_t ManagedRuntime::shared_string_allocation_size() const noexcept {
    std::size_t bytes = 0;
    for (const auto& entry : strings_)
        bytes += string_overhead + entry.second->size();
    return bytes;
}

std::size_t ManagedRuntime::sweep_shared_strings() {
    std::size_t bytes = 0;
    for (auto it = strings_.begin(); it != strings_.end();) {
        if (it->second.use_count() == 1) {
            bytes += string_overhead + it->second->size();
            it = strings_.erase(it);
        } else {
            ++it;
        }
    }
    return bytes;
}
