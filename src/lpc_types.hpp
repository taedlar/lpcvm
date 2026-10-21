// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
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

/**
 * @brief A shared pointer implementation for LPC compound data types's
 * reference semantics (strong reference only).
 *
 * This is a smart pointer that manages the lifetime of LPC compound data
 * types using reference counting with a co-allocated prefix control block.
 *
 * The std::shared_ptr is too expensive for a virtual machine's evaluation
 * stack due to its size overhead and separate allocation of the control
 * block, so we use a cheaper alternative with a co-allocated prefix control
 * block. Weak references (LpcHandle) share the control block's weak_count.
 * Thread-confined: counts are plain integers, so every LpcSharedPtr and
 * LpcHandle to the same object must be used on a single VM worker thread.
 * Cross-thread hand-off must go through plain data, never these pointers.
 */
template <typename T, typename RefType>
struct LpcHandle;

template <typename T>
struct LpcSharedPtr;

template <typename T, typename... Args>
LpcSharedPtr<T> lpc_make_shared(Args&&... args);

template <typename T>
struct LpcSharedPtr {
    T* ptr;

    struct ControlBlock { // the size of control block must be aligned to avoid undefined behavior
        uint32_t ref_count;
        uint32_t weak_count;
    };

private:
    template <typename U, typename R>
    friend struct LpcHandle;

    template <typename U, typename... Args>
    friend LpcSharedPtr<U> lpc_make_shared(Args&&... args);

    struct AdoptRetainedReference {};

    LpcSharedPtr(T* existing, AdoptRetainedReference) noexcept : ptr(existing) {}

    static ControlBlock* control_block(T* obj) noexcept {
        return reinterpret_cast<ControlBlock*>(obj) - 1;
    }

    static LpcSharedPtr retain(T* existing) {
        if (existing) {
            ++control_block(existing)->ref_count;
        }
        return LpcSharedPtr(existing, AdoptRetainedReference{});
    }

    static void release_weak(ControlBlock* cb) noexcept {
        if (--cb->weak_count == 0 && cb->ref_count == 0) {
            ::operator delete(cb);
        }
    }

    static void release_strong(T* obj) noexcept {
        ControlBlock* cb = control_block(obj);
        if (--cb->ref_count == 0) {
            ++cb->weak_count; // keep the block alive while the destructor runs
            obj->~T();
            release_weak(cb);
        }
    }

public:
    LpcSharedPtr() noexcept : ptr(nullptr) {}

    LpcSharedPtr(const LpcSharedPtr& other) {
        ptr = other.ptr;
        if (ptr) {
            ++control_block(ptr)->ref_count;
        }
    }

    LpcSharedPtr(LpcSharedPtr&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}

    LpcSharedPtr& operator=(const LpcSharedPtr& other) {
        if (this != &other) {
            if (ptr) {
                release_strong(ptr);
            }
            ptr = other.ptr;
            if (ptr) {
                ++control_block(ptr)->ref_count;
            }
        }
        return *this;
    }
    ~LpcSharedPtr() {
        if (ptr) {
            release_strong(ptr);
        }
    }

    uint32_t use_count() const noexcept {
        return ptr ? control_block(ptr)->ref_count : 0;
    }

    T* operator->() const {
        return ptr;
    }
    T& operator*() const {
        return *ptr;
    }
};

template <typename T, typename... Args>
LpcSharedPtr<T> lpc_make_shared(Args&&... args) {
    using ControlBlock = typename LpcSharedPtr<T>::ControlBlock;
    static_assert(alignof(T) <= sizeof(ControlBlock), "object would be misaligned after the prefix control block");
    void* mem = ::operator new(sizeof(ControlBlock) + sizeof(T)); // co-allocated control block and object
    ControlBlock* cb = new (mem) ControlBlock{1, 0};
    T* obj;
    try {
        obj = new (static_cast<void*>(cb + 1)) T(std::forward<Args>(args)...);
    } catch (...) {
        ::operator delete(mem);
        throw;
    }
    return LpcSharedPtr<T>(obj, typename LpcSharedPtr<T>::AdoptRetainedReference{});
}

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

/**
 * @brief Weak reference to an LpcSharedPtr-managed object, sharing the
 * object's prefix control block (weak_count).
 */
template <typename T, typename RefType>
struct LpcHandle {
    using ControlBlock = typename RefType::ControlBlock;

    T* ptr = nullptr;

    LpcHandle() noexcept = default;
    explicit LpcHandle(T* obj) noexcept : ptr(obj) {
        if (ptr) {
            ++RefType::control_block(ptr)->weak_count;
        }
    }
    LpcHandle(const LpcHandle& other) noexcept : ptr(other.ptr) {
        if (ptr) {
            ++RefType::control_block(ptr)->weak_count;
        }
    }
    LpcHandle(LpcHandle&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}
    LpcHandle& operator=(const LpcHandle& other) noexcept {
        LpcHandle tmp(other);
        std::swap(ptr, tmp.ptr);
        return *this;
    }
    LpcHandle& operator=(LpcHandle&& other) noexcept {
        LpcHandle tmp(std::move(other));
        std::swap(ptr, tmp.ptr);
        return *this;
    }
    ~LpcHandle() {
        if (ptr) {
            RefType::release_weak(RefType::control_block(ptr));
        }
    }

    uint32_t use_count() const noexcept {
        return ptr ? RefType::control_block(ptr)->ref_count : 0;
    }

    uint32_t weak_count() const noexcept {
        return ptr ? RefType::control_block(ptr)->weak_count : 0;
    }

    bool expired() const noexcept {
        return !ptr || RefType::control_block(ptr)->ref_count == 0;
    }

    /**
     * @brief Obtains a strong reference to the observed object.
     * @return The strong reference if the object is still alive, or
     * std::nullopt if it has been released.
     */
    std::optional<RefType> lock() const {
        if (expired()) {
            return std::nullopt;
        }
        return RefType::retain(ptr);
    }
};

class RuntimeClass;

class LpcObject {
public:
    using HandleType = LpcHandle<LpcObject, LpcObjectRef>;
    static HandleType create(std::string_view name, std::shared_ptr<RuntimeClass> program) {
        auto obj_ref = lpc_make_shared<LpcObject>(program);
        named_collection_.insert_or_assign(std::string(name), obj_ref);
        return obj_ref->handle();
    }

    LpcObject() = delete;
    LpcObject(const LpcObject&) = delete;
    LpcObject& operator=(const LpcObject&) = delete;
    ~LpcObject() = default;

    HandleType handle() const {
        return HandleType(const_cast<LpcObject*>(this));
    }

private:
    friend LpcSharedPtr<LpcObject>;
    template <typename U, typename... Args>
    friend LpcSharedPtr<U> lpc_make_shared(Args&&... args);
    LpcObject(std::shared_ptr<RuntimeClass> program)
        : program_(std::move(program)) {};

    std::shared_ptr<RuntimeClass> program_;
    // TODO: Implement the internal storage and methods for LpcObject

    static std::unordered_map<std::string, LpcObjectRef> named_collection_; // named references to LpcObject instances
};
