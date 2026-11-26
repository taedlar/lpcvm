// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <new>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

struct ManagedRuntime;
class LpcContainer;
class LpcObject;

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

struct SharedControlBlock {
    uint32_t ref_count;
    uint32_t weak_count;
    void (*destroy)(void*) noexcept;
};

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
    using element_type = T;

    template <typename>
    friend struct SharedPtr;

    using ControlBlock = SharedControlBlock;

    SharedPtr() noexcept : ptr(nullptr) {}

    // Deduce the optional's type to avoid recursively checking SharedPtr's
    // copy constructibility through optional's converting constructors.
    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    SharedPtr(std::optional<SharedPtr<U>> other) noexcept
        : SharedPtr(other ? std::move(*other) : SharedPtr{}) {}

    SharedPtr(const SharedPtr& other) {
        ptr = other.ptr;
        if (ptr) {
            ++control_block(ptr)->ref_count;
        }
    }

    template <typename U,
              std::enable_if_t<!std::is_same_v<T, U> && std::is_convertible_v<U*, T*>, int> = 0>
    SharedPtr(const SharedPtr<U>& other) {
        ptr = static_cast<T*>(other.ptr);
        if (ptr) {
            if (static_cast<const void*>(ptr) != static_cast<const void*>(other.ptr))
                throw std::runtime_error("SharedPtr only supports base classes with zero offset");
            ++SharedPtr<U>::control_block(other.ptr)->ref_count;
        }
    }

    SharedPtr(SharedPtr&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}

    template <typename U,
              std::enable_if_t<!std::is_same_v<T, U> && std::is_convertible_v<U*, T*>, int> = 0>
    SharedPtr(SharedPtr<U>&& other) {
        ptr = static_cast<T*>(other.ptr);
        if (ptr && static_cast<const void*>(ptr) != static_cast<const void*>(other.ptr))
            throw std::runtime_error("SharedPtr only supports base classes with zero offset");
        other.ptr = nullptr;
    }

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

    template <typename U,
              std::enable_if_t<!std::is_same_v<T, U> && std::is_convertible_v<U*, T*>, int> = 0>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        SharedPtr tmp(other);
        std::swap(ptr, tmp.ptr);
        return *this;
    }

    template <typename U,
              std::enable_if_t<!std::is_same_v<T, U> && std::is_convertible_v<U*, T*>, int> = 0>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        SharedPtr tmp(std::move(other));
        std::swap(ptr, tmp.ptr);
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

    void reset() noexcept {
        if (ptr) {
            release_strong(ptr);
            ptr = nullptr;
        }
    }

    template <typename U>
    SharedPtr<U> dynamic_pointer_cast() const {
        auto* cast = dynamic_cast<U*>(ptr);
        if (cast && static_cast<const void*>(cast) != static_cast<const void*>(ptr))
            throw std::runtime_error("SharedPtr only supports casts with zero offset");
        return SharedPtr<U>::retain(cast);
    }

private:
    T* ptr;

    template <typename U, typename R>
    friend struct Handle;

    template <typename U, typename... Args>
    friend SharedPtr<U> create_shared(Args&&... args);

    friend struct ::ManagedRuntime;
    friend class ::LpcContainer;
    friend class ::LpcObject;

    struct AdoptRetainedReference {};

    SharedPtr(T* existing, AdoptRetainedReference) noexcept : ptr(existing) {} // private constructor for allocating factories

    static ControlBlock* control_block(T* obj) noexcept {
        auto* bytes = reinterpret_cast<std::byte*>(obj);
        return std::launder(reinterpret_cast<ControlBlock*>(bytes - sizeof(ControlBlock)));
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
            cb->destroy(static_cast<void*>(cb + 1));
            release_weak(cb);
        }
    }
};

template <typename T>
inline void shared_ptr_destroy(void* obj) noexcept {
    static_cast<T*>(obj)->~T();
}

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
    static_assert(sizeof(ControlBlock) % alignof(T) == 0, "object would be misaligned after the prefix control block");
    void* mem = ::operator new(sizeof(ControlBlock) + sizeof(T)); // co-allocated control block and object
    ControlBlock* cb = new (mem) ControlBlock{1, 0, &shared_ptr_destroy<T>};
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
    Handle(const RefType& ref) noexcept : ptr(ref.ptr) {
        if (ptr) {
            ++RefType::control_block(ptr)->weak_count;
        }
    }

    template <typename U,
              std::enable_if_t<!std::is_same_v<T, U> && std::is_convertible_v<U*, T*>, int> = 0>
    Handle(const SharedPtr<U>& ref)
        : ptr(static_cast<T*>(ref.ptr)) {
        if (ptr) {
            if (static_cast<const void*>(ptr) != static_cast<const void*>(ref.ptr))
                throw std::runtime_error("Handle only supports base classes with zero offset");
            ++SharedPtr<U>::control_block(ref.ptr)->weak_count;
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

    bool operator<(const Handle& other) const noexcept {
        return std::less<T*>{}(ptr, other.ptr);
    }

    operator bool() const noexcept {
        return !expired();
    }

    const void* identity() const noexcept {
        return static_cast<const void*>(ptr);
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

/**
 * @brief A strong reference to an object recycled through Pool (no weak handles).
 *
 * Like SharedPtr, the object follows a co-allocated prefix control block and
 * counts are thread-confined. While referenced, the control block points to
 * the owning pool; when the last reference is released, Pool::recycle()
 * either links it into a free list or frees it.
 *
 * PooledPtr is usually used with copy-on-write semantics, where multiple references
 * can share the same underlying object until a modification is needed, at which
 * point a new copy is created.
 */
template <typename T, typename Pool>
struct PooledPtr {
    struct ControlBlock {
        uint32_t ref_count;
        union {
            Pool* pool;         // while referenced
            ControlBlock* next; // while recycled
        };
    };

    PooledPtr() noexcept : ptr(nullptr) {}
    PooledPtr(const PooledPtr& other) noexcept : ptr(other.ptr) {
        if (ptr) {
            ++control_block(ptr)->ref_count;
        }
    }
    PooledPtr(PooledPtr&& other) noexcept : ptr(std::exchange(other.ptr, nullptr)) {}
    PooledPtr& operator=(PooledPtr other) noexcept {
        std::swap(ptr, other.ptr);
        return *this;
    }
    ~PooledPtr() {
        if (ptr) {
            release(ptr);
        }
    }

    uint32_t use_count() const noexcept {
        return ptr ? control_block(ptr)->ref_count : 0;
    }

    bool operator==(const PooledPtr& other) const noexcept {
        return ptr == other.ptr;
    }

    operator bool() const noexcept {
        return ptr != nullptr;
    }

    T* operator->() const {
        if (!ptr) {
            throw std::runtime_error("Dereferencing null PooledPtr");
        }
        return ptr;
    }
    T& operator*() const {
        if (!ptr) {
            throw std::runtime_error("Dereferencing null PooledPtr");
        }
        return *ptr;
    }

private:
    T* ptr;

    friend Pool;

    struct AdoptRetainedReference {};

    PooledPtr(T* existing, AdoptRetainedReference) noexcept : ptr(existing) {}

    static ControlBlock* control_block(T* obj) noexcept {
        return reinterpret_cast<ControlBlock*>(obj) - 1;
    }

    static void release(T* obj) noexcept {
        ControlBlock* cb = control_block(obj);
        if (--cb->ref_count == 0) {
            Pool::recycle(cb);
        }
    }
};

class DataStack;
class ReturnStack;

class Object {
public:
    using FabricType = ManagedRuntime; // a fabric of substantial resources for executing LPC objects

    Object(std::shared_ptr<FabricType> sb)
        : weak_fabric_(sb) {}
    virtual ~Object() = default;

    /**
     * @brief Get the name of the object.
     * @return The name of the object as a string view, or an empty string view if the object has no name.
     */
    virtual std::string_view name() const { return std::string_view{}; }

    /**
     * @brief Call a method function on the object by its identifier.
     *   In LPCVM, if both caller and callee are bytecode-compiled objects, the call is optimized with the F_CALL_OTHER opcode.
     * @param func_identity A pointer identifying the function to call (e.g. a fabric-interned shared string).
     * @param data_stack The data stack for argument passing and return values.
     * @param return_stack The return stack for managing call frames.
     * @param num_args The number of arguments passed to the function.
     * @return The result of the function call, or -1 if the function calling fails or is not implemented for this object.
     *   If the function is successfully called, arguments on the data stack should be consumed and the return
     *   value should reflect the outcome of the call: 0 for no return value, positive for number of return values.
     *   If function calling fails or is not implemented, the data stack and return stack should remain unchanged. The caller
     *   is responsible for checking the return value to determine if the call was successful.
     */
    virtual int call_method (void* func_identity, DataStack* data_stack, ReturnStack* return_stack, int num_args) { return -1; }

    std::shared_ptr<FabricType> retain_fabric() const noexcept { return weak_fabric_.lock(); }

private:
    std::weak_ptr<FabricType> weak_fabric_;
};

using ObjectRef = SharedPtr<Object>;
using ObjectHandle = Handle<Object, SharedPtr<Object>>;

} // namespace lpcvm
