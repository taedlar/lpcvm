// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstddef>
#include <optional>
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
 * block. LPC compound data types does not require strong reference and
 * weak reference semantics at the same managed value, so we can use a
 * cheaper alternative with a co-allocated prefix control block.
 */
template <typename T, class... Args>
struct LpcSharedPtr {
    T* ptr;

    struct ControlBlock { // the size of control block must be aligned to avoid undefined behavior
        std::size_t ref_count;
    };

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

class LpcObject {
    // TODO: Implement the internal storage and methods for LpcObject
};

// weak reference to LpcObject using a generational handle
struct LpcObjectRef {
    uint32_t index;
    uint32_t generation;

    static LpcObjectRef create();
    void destroy() const;

    // RAII wrapper for strong references to LpcObject
    class StrongRef {
    private:
        uint32_t index;
        StrongRef(uint32_t idx); // private constructor, only constructible via LpcObjectRef::lock
        friend class LpcObjectRef;
    public:
        ~StrongRef();
    
        StrongRef(const StrongRef&) = delete; // prevent copying
        StrongRef& operator=(const StrongRef&) = delete; // prevent assignment
        StrongRef(StrongRef&&) = delete; // prevent accidental transfers of ownership

        LpcObject* operator->() const;
        LpcObject& operator*() const;
    };

    std::optional<LpcObject> get() const;
    std::optional<StrongRef> lock() const; // upgrade to strong reference if the object is still alive
};
