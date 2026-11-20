// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstddef>
#include <cstdint>
#include <new>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include "vm_object.hpp"

class LpcString;
class LpcBuffer;
class LpcBufferPool;
class LpcContainer;
class LpcArray;
class LpcMapping;
class LpcObject;

/**
 * @brief The lpcvm namespace contains all core components of the LPC virtual machine.
 */
namespace lpcvm {

struct svalue_t; // forward declaration of struct wrapper for SValue (stack value)
struct svalue_pair_t; // forward declaration of struct wrapper for a pair of SValues

} // namespace lpcvm

using LpcStringRef = lpcvm::SharedPtr<LpcString>;

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
    friend struct ManagedRuntime;

    explicit LpcString(std::size_t length) noexcept : length_(length) {}

    const std::size_t length_;
};

using LpcBufferRef = lpcvm::PooledPtr<LpcBuffer, LpcBufferPool>;

/** Mutable bytes recycled through the fabric's LpcBufferPool. Acquire one with
 * ManagedRuntime::make_buffer(); copies of LpcBufferRef share the same bytes.
 */
class LpcBuffer {
public:
    LpcBuffer(const LpcBuffer&) = delete;
    LpcBuffer& operator=(const LpcBuffer&) = delete;

    std::string data;

private:
    friend class LpcBufferPool;

    LpcBuffer() noexcept = default;
    ~LpcBuffer() = default;
};

namespace lpcvm {

struct BufferPoolLimits {
    std::size_t max_small_buffers = 1024;
    std::size_t max_large_buffers = 64;
    std::size_t max_large_capacity = 64 * 1024; // larger buffers are freed on release
};

struct BufferPoolStats {
    std::size_t live_buffers = 0;
    std::size_t small_buffers = 0;  // recycled, using std::string's inline storage
    std::size_t large_buffers = 0;  // recycled, retaining heap capacity
    std::size_t large_capacity = 0; // retained string capacity of recycled large buffers
};

} // namespace lpcvm

/**
 * @brief Recycles LpcBuffer allocations for one fabric.
 *
 * Small buffers fit in std::string's inline (SSO) storage; large buffers keep
 * their heap capacity in a separate free list. Released buffers are cleared
 * before recycling. Live buffers retain the pool, so they may outlive the
 * fabric; after close(), released buffers are freed instead of recycled.
 * Thread-confined.
 */
class LpcBufferPool {
public:
    using ControlBlock = LpcBufferRef::ControlBlock;

    LpcBufferPool(const LpcBufferPool&) = delete;
    LpcBufferPool& operator=(const LpcBufferPool&) = delete;

    /** Capacity of an empty std::string, which needs no heap allocation. */
    static std::size_t small_capacity() noexcept;

    /** Create a pool holding one owner reference, released by close(). */
    static LpcBufferPool* create(lpcvm::BufferPoolLimits limits = {});

    /** Acquire an empty buffer with at least `capacity` reserved bytes. */
    LpcBufferRef acquire(std::size_t capacity = 0);

    /** Free all recycled buffers; return their approximate allocation bytes. */
    std::size_t trim() noexcept;

    /** Trim, stop recycling, and release the owner reference. Call once. */
    void close() noexcept;

    lpcvm::BufferPoolStats stats() const noexcept;
    const lpcvm::BufferPoolLimits& limits() const noexcept { return limits_; }

private:
    template <typename U, typename P>
    friend struct lpcvm::PooledPtr;

    explicit LpcBufferPool(lpcvm::BufferPoolLimits limits) noexcept : limits_(limits) {}
    ~LpcBufferPool();

    static void recycle(ControlBlock* cb) noexcept;
    static LpcBuffer* buffer_of(ControlBlock* cb) noexcept;
    static ControlBlock* allocate(std::size_t capacity);
    static void destroy(ControlBlock* cb) noexcept;
    void unref() noexcept;

    lpcvm::BufferPoolLimits limits_;
    std::size_t ref_count_ = 1; // the owner plus each live buffer
    bool closed_ = false;
    ControlBlock* small_free_ = nullptr;
    ControlBlock* large_free_ = nullptr;
    lpcvm::BufferPoolStats stats_;
};

class LpcContainer {
public:
    // Borrowed, thread-confined owner; nullptr for untracked containers or after
    // runtime destruction. The returned pointer does not extend runtime lifetime.
    ManagedRuntime* fabric() const noexcept { return gc_owner_; }

protected:
    LpcContainer() = default;
    LpcContainer(const LpcContainer&) = delete;
    LpcContainer& operator=(const LpcContainer&) = delete;
    virtual ~LpcContainer();

    using EdgeVisitor = void (*)(LpcContainer*, void*);
    // Visit each owning container reference exactly once, including duplicates.
    // These hooks must not allocate, throw, or invoke collection recursively.
    virtual void visit_edges(EdgeVisitor visitor, void* context) const noexcept = 0;
    virtual void clear_edges() noexcept = 0;
    // Release the collector's temporary strong reference. This may destroy this.
    virtual void release_gc_reference() noexcept = 0;

    template <typename T>
    static void release_container_reference(T* container) noexcept {
        static_assert(std::is_base_of_v<LpcContainer, T>);
        lpcvm::SharedPtr<T>::release_strong(container);
    }

private:
    friend struct ManagedRuntime;
    ManagedRuntime* gc_owner_ = nullptr;
    uint32_t* gc_ref_count_ = nullptr;
    uint32_t gc_external_refs_ = 0;
    bool gc_marked_ = false;
    LpcContainer* gc_prev = nullptr; // previous container in the garbage collection list
    LpcContainer* gc_next = nullptr; // next container in the garbage collection list
};

using LpcArrayRef = lpcvm::SharedPtr<LpcArray>;

class LpcArray: public LpcContainer {
public:
    using FabricType = ManagedRuntime; // a fabric of substantial resources for executing LPC arrays

    static LpcArrayRef create(std::shared_ptr<FabricType> sb, std::size_t size = 0);

    LpcArray() = default;
    LpcArray(std::size_t size);
    ~LpcArray();

    std::size_t size() const noexcept { return size_; }

    void resize(std::size_t new_size);
    void clear() noexcept;
    lpcvm::svalue_t& operator[](std::size_t index) noexcept;
    const lpcvm::svalue_t& operator[](std::size_t index) const noexcept;

private:
    void visit_edges(EdgeVisitor visitor, void* context) const noexcept override;
    void clear_edges() noexcept override { clear(); }
    void release_gc_reference() noexcept override { release_container_reference(this); }

    uint32_t size_ = 0; // number of elements in the array
    uint32_t capacity_ = 0; // allocated capacity of the array
    lpcvm::svalue_t* values_ = nullptr; // pointer to the array elements
};

using LpcMappingRef = lpcvm::SharedPtr<LpcMapping>;

class LpcMapping: public LpcContainer {
public:
    using FabricType = ManagedRuntime; // a fabric of substantial resources for executing LPC mappings

    static LpcMappingRef create(std::shared_ptr<FabricType> sb, std::size_t size = 0);
    // Strings/buffers hash by bytes; numbers retain their type; other references use identity.
    // For TemporaryString, resolve the stack-owned bytes and call string_hash instead.
    static std::size_t hash(const lpcvm::svalue_t& key) noexcept;
    static std::size_t string_hash(std::string_view value) noexcept;

    LpcMapping() = default;
    explicit LpcMapping(std::size_t size); // reserve capacity; use a factory to attach a fabric
    ~LpcMapping();

    std::size_t size() const noexcept { return size_; }
    void clear() noexcept;

    // Expose only mapped values; stored keys remain private. Missing keys return nullptr.
    // Inserting a new key may invalidate all returned pointers and references.
    lpcvm::svalue_t* find(const lpcvm::svalue_t& key) noexcept;
    const lpcvm::svalue_t* find(const lpcvm::svalue_t& key) const noexcept;
    // Return an existing value or insert a monostate value; failures throw.
    // Buffer keys are interned through the fabric to preserve immutable key bytes.
    lpcvm::svalue_t& find_or_insert(const lpcvm::svalue_t& key);
    lpcvm::svalue_t& operator[](const lpcvm::svalue_t& key) {
        return find_or_insert(key);
    }
    // Missing keys are a no-op. Erasure invalidates references to the erased value
    // and the last entry, which may move into its slot. Temporary keys are rejected.
    void erase(const lpcvm::svalue_t& key);

private:
    void visit_edges(EdgeVisitor visitor, void* context) const noexcept override;
    void clear_edges() noexcept override { clear(); }
    void release_gc_reference() noexcept override { release_container_reference(this); }

    bool need_grow() const noexcept {
        return bucket_bits_ == 0 || size_ >= (uint32_t{1} << (bucket_bits_ - 1));
    }
    uint32_t append(const lpcvm::svalue_pair_t& entry); // append a new entry to the mapping
    std::optional<uint32_t> find_index(const lpcvm::svalue_t& key) const noexcept;
    void grow(); // rebuild the bucket table, widening indices as needed
    void rehash(uint32_t bucket_bits);

    constexpr static std::size_t ENTRY_CHUNK_SIZE = 8; // number of entries to allocate at a time in chunks
    constexpr static int IX_EMPTY = -1;
    constexpr static int IX_ERASED = -2;
    constexpr static uint32_t MAX_ENTRIES = uint32_t{1} << 30;

    uint32_t size_ = 0; // number of entries in the mapping
    uint32_t capacity_ = 0; // allocated entry slots
    uint32_t erased_ = 0; // deleted buckets, reused by insertion or removed by rehashing
    uint32_t bucket_bits_ = 0; // number of bits used for the hash table buckets (2^bucket_bits_ = number of buckets)
    std::variant<std::monostate, int8_t*, int16_t*, int32_t*> indices_; // hash table buckets for the mapping
    lpcvm::svalue_pair_t* entries_ = nullptr; // pointer to the mapping entries
};

using LpcObjectRef = lpcvm::SharedPtr<LpcObject>; // strong reference to LpcObject instances
using LpcObjectHandle = lpcvm::Handle<LpcObject, LpcObjectRef>;

class ObjectClass;
class ObjectState;

class LpcObject: public lpcvm::Object {
public:
    using HandleType = lpcvm::ObjectHandle;

    static HandleType create(std::shared_ptr<FabricType> sb, std::string_view name, std::shared_ptr<ObjectClass> program);

    LpcObject() = delete;
    LpcObject(const LpcObject&) = delete;
    LpcObject& operator=(const LpcObject&) = delete;
    ~LpcObject();

    std::string_view name() const override { return name_ ? name_->view() : std::string_view{}; }
    ObjectState* state() const noexcept { return state_.get(); }
    std::shared_ptr<ObjectClass> program() const noexcept { return program_; }

private:
    template <typename U, typename... Args>
    friend lpcvm::SharedPtr<U> lpcvm::create_shared(Args&&... args); // co-allocator to create prefix control block and the object

    LpcObject(std::shared_ptr<FabricType> sb, std::shared_ptr<ObjectClass> program); // private ctor: use LpcObject::create() instead

    LpcStringRef name_;
    std::unique_ptr<ObjectState> state_;
    std::shared_ptr<ObjectClass> program_;
};
