// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include "lpc_types.hpp"
#include "managed_runtime.hpp"

using ObjectTable = std::unordered_map<std::string_view, LpcObjectRef>;
using ObjectHandle = LpcObject::HandleType;
using PathType = std::filesystem::path;

namespace lpcvm {

class Sandbox: public ManagedRuntime {
public:

    Sandbox(const PathType &mudlib_root)
        : mudlib_root_(mudlib_root) {}

    // Paths (including /absolute paths) are relative to the mudlib root.
    // The path must exist, including for write modes.
    // Returns nullptr and sets errno on failure. Symlinks must resolve inside
    // the root. The host must prevent concurrent filesystem changes between
    // path validation and opening; this is not an OS-level filesystem jail.
    FILE* file_open(const char* path, const char* mode) override;

    /**
     * @brief Get the collection of all objects in the sandbox.
     *
     * All objects in the sandbox can be found with an unique name.
     * Objects loaded through the sandbox use normalized mudlib-relative paths
     * with forward slashes, no leading slash, and no source file extension
     * (for example, std/player).
     * If the object name contains a `#` character, it is a cloned object.
     * Cloned objects are typically created via the `clone_object` method.
     * Otherwise, the object is a blueprint object that can be cloned to
     * create new instances.
     *
     * Once an object is loaded or cloned, the sandbox maintains a reference
     * to it in the object table until it is explicitly destructed with
     * the `destruct_object` method or in the sandbox's destructor.
     *
     * In LPC, objects are always passed around by handle (weak references).
     * This allows the sandbox to represent any external resource as an
     * object in the sandbox without taking ownership of it. For example,
     * file handles or network connections can be represented as objects
     * without the sandbox being responsible for their lifetime.
     */
    const ObjectTable& objects() const { return named_collection_; }

    ObjectHandle load_object (const PathType &path);
    ObjectHandle find_object (std::string_view name);
    ObjectHandle clone_object (ObjectHandle obj);
    void destruct_object (ObjectHandle obj);

    friend class ::LpcObject; // to allow adding/removing objects from the sandbox's object table

private:
    PathType mudlib_root_;
    ObjectTable named_collection_; // named references to LpcObject instances

    ObjectTable& objects() { return named_collection_; }
};

} // namespace lpcvm
