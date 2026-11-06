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

class ManagedRuntime;
class LpcString;
class LpcArray;
class LpcMapping;
class LpcObject;

/**
 * @brief The lpcvm namespace contains all core components of the LPC virtual machine.
 */
namespace lpcvm {

/**
 * @brief A weak reference to an LPC compound data type managed by SharedPtr.
 *
 * Handle does not own the object; it only observes it. The object may be
 * destroyed if all SharedPtr instances are released, at which point the
 * Handle becomes expired.
 */
template <typename T, typename RefType>
struct Handle;

/**
 * @brief A shared pointer implementation for LPC compound data types'
 * reference semantics (strong reference only).
 *
 * This is a smart pointer that manages the lifetime of LPC compound data
 * types using reference counting with a co-allocated prefix control block.
 *
 * The std::shared_ptr is too expensive for a virtual machine's data
 * stack due to its size overhead and separate allocation of the control
 * block, so we use a cheaper alternative with a co-allocated prefix control
 * block. Weak references (Handle) share the control block's weak_count.
 * Thread-confined: counts are plain integers, so every SharedPtr and
 * Handle to the same object must be used on a single VM worker thread.
 * Cross-thread hand-off must go through plain data, never these pointers.
 */
template <typename T>
struct SharedPtr {

    struct ControlBlock { // the size of control block must be aligned to avoid undefined behavior
        uint32_t ref_count;
        uint32_t weak_count;
    };

    SharedPtr() noexcept : ptr(nullptr) {}

    SharedPtr(const SharedPtr& other) {
        ptr = other.ptr;
        if (ptr) {
            ++control_block(ptr)->ref_count;
        }
    }

    SharedPtr(SharedPtr&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}

    SharedPtr& operator=(const SharedPtr& other) {
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
    ~SharedPtr() {
        if (ptr) {
            release_strong(ptr);
        }
    }

    uint32_t use_count() const noexcept {
        return ptr ? control_block(ptr)->ref_count : 0;
    }

    bool operator==(const SharedPtr& other) const noexcept {
        return ptr == other.ptr;
    }

    operator bool() const noexcept {
        return ptr != nullptr;
    }

    T* operator->() const {
        if (!ptr) {
            throw std::runtime_error("Dereferencing null SharedPtr");
        }
        return ptr;
    }
    T& operator*() const {
        if (!ptr) {
            throw std::runtime_error("Dereferencing null SharedPtr");
        }
        return *ptr;
    }

private:
    T* ptr;

    template <typename U, typename R>
    friend struct Handle;

    template <typename U, typename... Args>
    friend SharedPtr<U> create_shared(Args&&... args);

    friend class ::ManagedRuntime;

    struct AdoptRetainedReference {};

    SharedPtr(T* existing, AdoptRetainedReference) noexcept : ptr(existing) {} // private constructor for allocating factories

    static ControlBlock* control_block(T* obj) noexcept {
        return reinterpret_cast<ControlBlock*>(obj) - 1;
    }

    static SharedPtr retain(T* existing) {
        if (existing) {
            ++control_block(existing)->ref_count;
        }
        return SharedPtr(existing, AdoptRetainedReference{});
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
};

/**
 * @brief Co-allocates a control block and an object, returning an SharedPtr managing it.
 * @tparam T The type of the object to create.
 * @tparam Args The types of the arguments to forward to T's constructor.
 * @param args The arguments to forward to T's constructor.
 * @return An SharedPtr managing the newly created object.
 * @throws std::bad_alloc if memory allocation fails.
 * @throws Any exception thrown by T's constructor.
 */
template <typename T, typename... Args>
SharedPtr<T> create_shared(Args&&... args) {
    using ControlBlock = typename SharedPtr<T>::ControlBlock;
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
    return SharedPtr<T>(obj, typename SharedPtr<T>::AdoptRetainedReference{});
}

/**
 * @brief Weak reference to an SharedPtr-managed object, sharing the
 * object's prefix control block (weak_count).
 */
template <typename T, typename RefType>
struct Handle {
    using ControlBlock = typename RefType::ControlBlock;

    Handle() noexcept : ptr(nullptr) {}
    Handle(RefType& ref) noexcept : ptr(ref.ptr) {
        if (ptr) {
            ++RefType::control_block(ptr)->weak_count;
        }
    }

    Handle(const Handle& other) noexcept : ptr(other.ptr) {
        if (ptr) {
            ++RefType::control_block(ptr)->weak_count;
        }
    }
    Handle(Handle&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}
    Handle& operator=(const Handle& other) noexcept {
        Handle tmp(other);
        std::swap(ptr, tmp.ptr);
        return *this;
    }
    Handle& operator=(Handle&& other) noexcept {
        Handle tmp(std::move(other));
        std::swap(ptr, tmp.ptr);
        return *this;
    }
    ~Handle() {
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

    bool operator==(const Handle& other) const noexcept {
        return ptr == other.ptr;
    }

    operator bool() const noexcept {
        return !expired();
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

private:
    T* ptr = nullptr;

    /**
     * @brief Constructs a weak handle from a raw pointer to the object.
     * @param obj The raw pointer to the object.
     * @note This constructor is private and should only be used internally.
     */
    explicit Handle(T* obj) noexcept : ptr(obj) {
        if (ptr) {
            ++RefType::control_block(ptr)->weak_count;
        }
    }
};

} // namespace lpcvm

using LpcStringRef = lpcvm::SharedPtr<LpcString>;

namespace lpcvm {

/** Find or create an immutable owning string, interned within the runtime.
 * Like SharedPtr, the runtime and returned references are thread-confined.
 * The input is copied on creation and may contain embedded NUL bytes.
 * Unused strings are retained until the runtime sweeps them or is destroyed.
 */
LpcStringRef make_shared_string(ManagedRuntime& runtime, std::string_view value);

} // namespace lpcvm

/** Immutable bytes stored immediately after this object, in the same allocation
 * as the SharedPtr control block. Only ManagedRuntime can construct one.
 */
class LpcString {
public:
    LpcString(const LpcString&) = delete;
    LpcString& operator=(const LpcString&) = delete;
    ~LpcString() = default;

    // Borrowed views remain valid while an owning reference is retained.
    std::string_view view() const noexcept { return {data(), length_}; }
    const char* data() const noexcept { return reinterpret_cast<const char*>(this + 1); }
    const char* c_str() const noexcept { return data(); }
    std::size_t size() const noexcept { return length_; }
    bool empty() const noexcept { return length_ == 0; }

private:
    friend class ManagedRuntime;

    explicit LpcString(std::size_t length) noexcept : length_(length) {}

    const std::size_t length_;
};

using LpcArrayRef = lpcvm::SharedPtr<LpcArray>;

class LpcArray {
    // TODO: Implement the internal storage and methods for LpcArray
};

using LpcMappingRef = lpcvm::SharedPtr<LpcMapping>;

class LpcMapping {
    // TODO: Implement the internal storage and methods for LpcMapping
};

using LpcObjectRef = lpcvm::SharedPtr<LpcObject>; // strong reference to LpcObject instances

class ObjectClass;
class ObjectState;

class LpcObject {
public:
    using FabricType = ManagedRuntime; // a fabric of substantial resources for executing LPC objects
    using HandleType = lpcvm::Handle<LpcObject, LpcObjectRef>;

    static HandleType create(FabricType *sb, std::string_view name, std::shared_ptr<ObjectClass> program);

    LpcObject() = delete;
    LpcObject(const LpcObject&) = delete;
    LpcObject& operator=(const LpcObject&) = delete;
    ~LpcObject() = default;

    FabricType *fabric() const noexcept { return fabric_; }
    // Owning reference; null for objects constructed without a registered name.
    LpcStringRef name() const { return name_; }
    std::shared_ptr<ObjectClass> program() const noexcept { return program_; }
    std::shared_ptr<ObjectState> state() const noexcept { return state_; }

private:
    template <typename U, typename... Args>
    friend lpcvm::SharedPtr<U> lpcvm::create_shared(Args&&... args);

    LpcObject(FabricType *sb, std::shared_ptr<ObjectClass> program); // private ctor: use LpcObject::create() instead

    FabricType *fabric_;
    LpcStringRef name_;
    std::shared_ptr<ObjectClass> program_;
    std::shared_ptr<ObjectState> state_;
};
