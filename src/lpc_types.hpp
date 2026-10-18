// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "vm_runtime_class.hpp"

/**
 * @brief A shared pointer implementation for LPC compound data types's
 * reference semantics (strong reference only).
 *
 * This is a smart pointer that manages the lifetime of LPC compound data
 * types using reference counting with a co-allocated prefix control block.
 *
 * The std::shared_ptr is too expensive for a virtual machine's evaluation
 * stack due to its size overhead and separate allocation of the control
 * block. LPC compound data types does not require strong reference and
 * weak reference semantics at the same managed value, so we can use a
 * cheaper alternative with a co-allocated prefix control block.
 * LPC references and object handles are confined to the VM worker thread.
 */
template <typename T>
struct LpcHandle;

template <typename T>
struct LpcSharedPtr {
    T* ptr;

    struct ControlBlock { // the size of control block must be aligned to avoid undefined behavior
        std::size_t ref_count;
    };

private:
    template <typename>
    friend struct LpcHandle;

    struct AdoptRetainedReference {};

    LpcSharedPtr(T* existing, AdoptRetainedReference) noexcept : ptr(existing) {}

    static LpcSharedPtr retain(T* existing) {
        if (existing) {
            ControlBlock* cb = reinterpret_cast<ControlBlock*>(existing) - 1;
            ++cb->ref_count;
        }
        return LpcSharedPtr(existing, AdoptRetainedReference{});
    }

public:
    template <class... Args, std::enable_if_t<!(sizeof...(Args) == 1 && (std::is_same_v<std::decay_t<Args>, LpcSharedPtr> || ...)), int> = 0>
    LpcSharedPtr(Args&&... args) {
        void* mem = ::operator new(sizeof(ControlBlock) + sizeof(T)); // co-allocated control block and object
        ControlBlock* cb = new (mem) ControlBlock{1};
        ptr = new (static_cast<void*>(cb + 1)) T(std::forward<Args>(args)...);
    }
    LpcSharedPtr(const LpcSharedPtr& other) {
        ptr = other.ptr;
        if (ptr) {
            ControlBlock* cb = reinterpret_cast<ControlBlock*>(ptr) - 1;
            ++cb->ref_count;
        }
    }

    LpcSharedPtr(LpcSharedPtr&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}

    LpcSharedPtr& operator=(const LpcSharedPtr& other) {
        if (this != &other) {
            if (ptr) {
                ControlBlock* cb = reinterpret_cast<ControlBlock*>(ptr) - 1;
                if (--cb->ref_count == 0) {
                    ptr->~T();
                    ::operator delete(cb);
                }
            }
            ptr = other.ptr;
            if (ptr) {
                ControlBlock* cb = reinterpret_cast<ControlBlock*>(ptr) - 1;
                ++cb->ref_count;
            }
        }
        return *this;
    }
    ~LpcSharedPtr() {
        if (ptr) {
            ControlBlock* cb = reinterpret_cast<ControlBlock*>(ptr) - 1;
            if (--cb->ref_count == 0) {
                ptr->~T();
                ::operator delete(cb);
            }
        }
    }

    T* operator->() const {
        return ptr;
    }
    T& operator*() const {
        return *ptr;
    }
};

class LpcArray {
    // TODO: Implement the internal storage and methods for LpcArray
};

class LpcMapping {
    // TODO: Implement the internal storage and methods for LpcMapping
};

using LpcMappingRef = LpcSharedPtr<LpcMapping>;
using LpcArrayRef = LpcSharedPtr<LpcArray>;

class LpcObject;
using LpcObjectRef = LpcSharedPtr<LpcObject>; // strong reference to LpcObject instances

template <typename T>
struct LpcHandle {
    /**
     * @brief Attempts to lock the handle and obtain a strong reference to
     * the observed object.
     * @return An optional containing the LpcObjectRef if the object is still
     * alive, or std::nullopt if it has been released.
     */
    std::optional<LpcSharedPtr<T>> lock() const {
        auto* slot = get_slot(index);
        if (!slot || slot->generation != generation || !slot->observe) {
            return std::nullopt;
        }
        return LpcSharedPtr<T>::retain(slot->observe);
    }

    static constexpr uint32_t invalid_index =
        std::numeric_limits<uint32_t>::max();

    uint32_t index;
    uint32_t generation;

    struct MapSlot {
        T* observe = nullptr; // a generational observer (weak reference)
        uint32_t generation = 0; // changes when observed object is replaced or released
        uint32_t next_free = invalid_index;
    };

    static constexpr uint32_t slots_per_chunk = 1U << 16;
    static constexpr std::size_t slot_chunk_count =
        (static_cast<uint64_t>(invalid_index) + slots_per_chunk - 1) / slots_per_chunk;

    inline static std::array<MapSlot*, slot_chunk_count> slot_chunks{}; // worker-confined weak-reference storage
    inline static uint32_t next_slot_index = 0;
    inline static uint32_t free_slot_head = invalid_index;

    static LpcHandle create_handle(T* obj) {
        if (!obj) {
            throw std::invalid_argument("Cannot create a handle for a null object");
        }

        auto index = pop_free_slot();
        if (index == invalid_index) {
            index = reserve_slot_index();
        }
        auto* slot = get_slot(index);
        const auto slot_generation = slot->generation;
        slot->observe = obj;
        return {index, slot_generation};
    }

    static void release_handle(LpcHandle handle) {
        if (handle.index == invalid_index) {
            return;
        }

        auto* slot = get_slot(handle.index);
        if (!slot || slot->generation != handle.generation) {
            return;
        }

        slot->observe = nullptr;
        ++slot->generation;
        push_free_slot(handle.index, *slot);
    }

private:
    static MapSlot* get_slot(uint32_t slot_index) noexcept {
        if (slot_index == invalid_index || slot_index >= next_slot_index) {
            return nullptr;
        }

        const auto chunk_index = slot_index / slots_per_chunk;
        auto* chunk = slot_chunks[chunk_index];
        return chunk ? &chunk[slot_index % slots_per_chunk] : nullptr;
    }

    static MapSlot* get_or_create_chunk_slot(uint32_t slot_index) {
        const auto chunk_index = slot_index / slots_per_chunk;
        auto*& chunk = slot_chunks[chunk_index];
        if (!chunk) {
            chunk = new MapSlot[slots_per_chunk];
        }
        return &chunk[slot_index % slots_per_chunk];
    }

    static uint32_t reserve_slot_index() {
        const auto slot_index = next_slot_index;
        if (slot_index == invalid_index) {
            throw std::length_error("LpcHandle slot capacity exhausted");
        }

        get_or_create_chunk_slot(slot_index);
        ++next_slot_index;
        return slot_index;
    }

    static uint32_t pop_free_slot() noexcept {
        const auto slot_index = free_slot_head;
        if (slot_index == invalid_index) {
            return slot_index;
        }

        auto* slot = get_slot(slot_index);
        free_slot_head = slot->next_free;
        return slot_index;
    }

    static void push_free_slot(uint32_t slot_index, MapSlot& slot) noexcept {
        slot.next_free = free_slot_head;
        free_slot_head = slot_index;
    }
};

class LpcObject {
public:
    using HandleType = LpcHandle<LpcObject>;
    static HandleType create(std::string_view name, std::shared_ptr<RuntimeClass> program) {
        LpcObjectRef obj_ref{program};
        named_collection_.insert_or_assign(std::string(name), obj_ref);
        return obj_ref->handle();
    }

    LpcObject() = delete;
    LpcObject(const LpcObject&) = delete;
    LpcObject& operator=(const LpcObject&) = delete;
    ~LpcObject() {
        HandleType::release_handle(handle_);
    };

    HandleType handle() const {
        return handle_;
    }

private:
    friend LpcSharedPtr<LpcObject>;
    LpcObject(std::shared_ptr<RuntimeClass> program)
        : handle_(HandleType::create_handle(this)), program_(std::move(program)) {};

    LpcHandle<LpcObject> handle_;
    std::shared_ptr<RuntimeClass> program_;
    // TODO: Implement the internal storage and methods for LpcObject

    static std::unordered_map<std::string, LpcObjectRef> named_collection_; // named references to LpcObject instances
};
