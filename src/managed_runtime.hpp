// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdio>
#include <string_view>
#include <unordered_map>
#include "lpc_types.hpp"

using ObjectTable = std::unordered_map<std::string_view, LpcObjectRef>;
using ObjectHandle = LpcObject::HandleType;

/** Default runtime services, usable without a Sandbox. Like SharedPtr, string
 * allocation, references, and sweeping are confined to the owning thread.
 */
class ManagedRuntime {
public:
    ManagedRuntime() = default;
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
    const ObjectTable& objects() const { return named_collection_; }

    /**
     * @brief Insert or update an object in the runtime's object table.
     * Returns the handle to the inserted or updated object.
     */
    virtual ObjectHandle emplace_object (std::string_view name, LpcObjectRef obj) {
        objects().erase(name); // always erase the string_view key if it exists
        objects().emplace(obj->name(), obj); // use the interned string_view as the key
        return ObjectHandle(obj);
    }

    /**
     * @brief Find an object by its unique name. Returns an empty handle if not found.
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
        if (obj_ref != std::nullopt) {
            ObjectTable::iterator it = objects().find((*obj_ref)->name());
            if (it != objects().end() && it->second == *obj_ref) {
                objects().erase(it); // also erase the string_view pointing to the object's name
            }
        }
    }

protected:
    /** 
     * @brief The mutable object table is used by emplace_object and destruct_object.
     * A derived class may use this to access and modify the object table directly.
     */
    ObjectTable& objects() { return named_collection_; }

    static LpcStringRef allocate_shared_string(std::string_view value);

private:
    ObjectTable named_collection_; // named references to LpcObject instances
    std::unordered_map<std::string_view, LpcStringRef> strings_;

};
