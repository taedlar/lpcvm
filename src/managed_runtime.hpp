// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdio>
#include <string_view>
#include <unordered_map>
#include "lpc_types.hpp"

using ObjectTable = std::unordered_map<std::string_view, LpcObjectRef>;
using ObjectHandle = LpcObject::HandleType;

/** Lightweight, scope-owned runtime services, usable without a Sandbox.
 * Destruction releases the tables' owning references. External owning references
 * may survive, but must not use an object's fabric pointer after this runtime dies.
 * The address is stable: objects and execution contexts borrow this runtime.
 * Like SharedPtr, allocation, references, and sweeping are thread-confined.
 */
struct ManagedRuntime {
    ManagedRuntime() = default;
    ManagedRuntime(const ManagedRuntime&) = delete;
    ManagedRuntime& operator=(const ManagedRuntime&) = delete;
    ManagedRuntime(ManagedRuntime&&) = delete;
    ManagedRuntime& operator=(ManagedRuntime&&) = delete;
    virtual ~ManagedRuntime() = default;

    virtual FILE* file_open(const char* path, const char* mode) {
        return std::fopen(path, mode);
    }

    /**
     * @brief Find or create an immutable owning string. Copies input bytes, including
     * embedded NULs. Returned strings can outlive this runtime.
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
     * @brief The immutable object table provides a view of all named objects
     * managed by this runtime. The table maps unique object names to their
     * corresponding LpcObject references.
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
    virtual ObjectHandle emplace_object (LpcObjectRef obj) {
        if (!obj)
            throw std::invalid_argument("Object cannot be null");
        const auto name = obj->name();
        if (!name || name->empty())
            throw std::invalid_argument("Object name cannot be empty");
        if (obj->fabric() != this)
            throw std::invalid_argument("Object belongs to another runtime");

        mutable_objects().erase(name->view()); // replace the borrowed key, not just its value
        mutable_objects().emplace(name->view(), obj);
        return ObjectHandle(obj);
    }

    /**
     * @brief Find an object by its unique name. Returns an empty reference if not found.
     */
    virtual LpcObjectRef find_object (std::string_view name) {
        auto obj_ref = objects().find(name);
        if (obj_ref == objects().end()) {
            return LpcObjectRef();
        }
        return obj_ref->second;
    }

    /**
     * @brief Destruct an object by its handle. Removes it from the runtime's object table.
     */
    virtual void destruct_object (ObjectHandle obj) {
        auto obj_ref = obj.lock();
        if (obj_ref != std::nullopt && (*obj_ref)->name()) {
            auto it = objects().find((*obj_ref)->name()->view());
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
    ObjectTable named_collection_; // named references to LpcObject instances
    std::unordered_map<std::string_view, LpcStringRef> strings_;

};
