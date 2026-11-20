// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdio>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "lpc_types.hpp"

namespace lpcvm {
    using PathType = std::filesystem::path;
}

/**
 * Lightweight, scope-bound runtime resource manager for LPCVM in RAII style.
 *
 * This is a prototype implementation for the LPCVM sandbox that
 * provides basic memory and resource management for polymorphic objects,
 * shared strings, recycled buffers, and garbage-collected containers.
 */
struct ManagedRuntime : public std::enable_shared_from_this<ManagedRuntime> {
    using ObjectTable = std::unordered_map<std::string_view, lpcvm::ObjectRef>;

    ManagedRuntime() = default;
    ManagedRuntime(const ManagedRuntime&) = delete;
    ManagedRuntime& operator=(const ManagedRuntime&) = delete;
    ManagedRuntime(ManagedRuntime&&) = delete;
    ManagedRuntime& operator=(ManagedRuntime&&) = delete;
    virtual ~ManagedRuntime();

    virtual FILE* file_open(const char* path, const char* mode) {
        return std::fopen(path, mode);
    }

    /** Find the first regular file in search_paths, in order. Relative names
     * require an explicit search directory (use "." for the current directory).
     * Absolute names are checked directly, ignoring search_paths. Returns a
     * canonical absolute path, or nullopt if no regular file exists. Missing
     * candidates are skipped; other filesystem errors propagate. Empty names
     * and paths containing embedded NULs throw std::invalid_argument.
     */
    virtual std::optional<lpcvm::PathType> file_search(
        const lpcvm::PathType& name,
        const std::vector<lpcvm::PathType>& search_paths);

    /** @brief Find an existing shared string, if any. Returns nullopt if not found. */
    virtual std::optional<LpcStringRef> find_shared_string(std::string_view value);

    /**
     * @brief Find or create an immutable owning string. Copies input bytes, including
     * embedded NULs. Returned strings can outlive this runtime. Unused strings are
     * retained until sweep_shared_strings() or destruction.
     */
    virtual LpcStringRef make_shared_string(std::string_view value);

    /**
     * @brief Bytes allocated for strings currently interned here, including reference
     * counts, metadata, and the trailing NUL. Excludes table/allocator overhead.
     * The default implementation scans the table in linear time.
     */
    virtual std::size_t shared_string_allocation_size() const noexcept;

    /**
     * @brief Remove strings owned only by the table; return their allocation bytes.
     * Linear time by default. Buckets are retained; external weak handles may
     * delay physical deallocation. Overrides with their own storage should
     * override allocation accounting and sweeping together.
     */
    virtual std::size_t sweep_shared_strings();

    /**
     * @brief Acquire an empty mutable buffer with at least `capacity` reserved
     * bytes from this runtime's pool. Buffers can outlive this runtime; those
     * released afterwards are freed rather than recycled.
     */
    LpcBufferRef make_buffer(std::size_t capacity = 0);

    lpcvm::BufferPoolStats buffer_pool_stats() const noexcept;

    /** @brief Free recycled buffers; return their approximate allocation bytes. */
    std::size_t trim_buffer_pool() noexcept;

    /** Create a shared container and track it without retaining ownership. */
    template <typename RefType, typename... Args>
    RefType make_container(Args&&... args) {
        using ContainerType = typename RefType::element_type;
        static_assert(std::is_base_of_v<LpcContainer, ContainerType>,
                      "make_container requires an LpcContainer type");
        auto ref = lpcvm::create_shared<ContainerType>(std::forward<Args>(args)...);
        auto* container = static_cast<LpcContainer*>(&*ref);
        container->gc_ref_count_ = &RefType::control_block(&*ref)->ref_count;
        container->gc_owner_ = this;
        container->gc_next = gc_head_;
        if (gc_head_)
            gc_head_->gc_prev = container;
        gc_head_ = container;
        return ref;
    }

    /** Reclaim unreachable containers allocated by this runtime; return their
     * count. External strong references are roots; weak handles are not.
     * Call explicitly on the owning thread while the graph is not being mutated.
     * References from untracked containers or other runtimes remain roots, so
     * cycles crossing runtime boundaries are not collected. Scratch allocation
     * failure leaves the graph unchanged. Surviving containers are detached on
     * runtime destruction and are no longer eligible for collection.
     */
    std::size_t collect_garbage();

    /**
     * @brief The immutable object table provides a view of all named objects
     * managed by this runtime. The table maps unique object names to their
     * corresponding lpcvm::Object references.
     */
    const ObjectTable& objects() const noexcept { return named_collection_; }

    /**
     * @brief Insert or update an object in the runtime's object table.
     * Returns the handle to the inserted or updated object.
     *
     * The key borrows the object's own name. Replacing an object also replaces
     * the borrowed key so its lifetime follows the new object.
     * @param obj A named object belonging to this runtime.
     * @throws std::invalid_argument If obj is null, has no non-empty name,
     *      or belongs to another runtime. Validation leaves the table unchanged.
     */
    virtual lpcvm::ObjectHandle emplace_object (lpcvm::ObjectRef obj) {
        if (!obj)
            throw std::invalid_argument("Object cannot be null");
        const auto name = obj->name();
        if (name.empty())
            throw std::invalid_argument("Object name cannot be empty");
        if (name.find(':') != std::string_view::npos)
            throw std::invalid_argument("Object name cannot contain ':'"); // ':' is reserved for URI objects
        if (obj->retain_fabric().get() != this)
            throw std::invalid_argument("Object belongs to another runtime");

        mutable_objects().erase(name); // replace the borrowed key, not just its value
        mutable_objects().emplace(name, obj);
        return lpcvm::ObjectHandle(obj);
    }

    /**
     * @brief Find an object by its unique name. Returns an empty reference if not found.
     */
    virtual lpcvm::ObjectHandle find_object (std::string_view name) {
        auto obj_ref = objects().find(name);
        if (obj_ref == objects().end()) {
            return lpcvm::ObjectHandle{};
        }
        return lpcvm::ObjectHandle(obj_ref->second);
    }

    /**
     * @brief Destruct an object by its handle. Removes it from the runtime's object table.
     */
    virtual void destruct_object (lpcvm::ObjectHandle obj) {
        auto obj_ref = obj.lock();
        if (obj_ref != std::nullopt && !(*obj_ref)->name().empty()) {
            auto it = objects().find((*obj_ref)->name());
            if (it != objects().end() && it->second == *obj_ref) {
                mutable_objects().erase(it); // also erase the string_view pointing to the object's name
            }
        }
    }

protected:
    /** 
     * @brief The mutable object table is used by emplace_object and destruct_object.
     * A derived class may use this to access and modify the object table directly.
     */
    ObjectTable& mutable_objects() noexcept { return named_collection_; }

    static LpcStringRef allocate_shared_string(std::string_view value);

private:
    friend class LpcContainer;
    ObjectTable named_collection_; // named references to lpcvm::Object instances (owned by the fabric)

    std::unordered_map<std::string_view, LpcStringRef> strings_; // owned strings keyed by their string_view
    LpcBufferPool* buffer_pool_ = nullptr; // created on first use, closed on destruction
    LpcContainer* gc_head_ = nullptr; // head of the garbage collection list for containers
};
