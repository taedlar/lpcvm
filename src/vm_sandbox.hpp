// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <filesystem>
#include <unordered_map>
#include "lpc_types.hpp"

namespace lpcvm {

using ObjectTable = std::unordered_map<std::string_view, LpcObjectRef>;
using ObjectHandle = LpcObject::HandleType;
using PathType = std::filesystem::path;

class Sandbox {
public:

    Sandbox(const PathType &mudlib_root)
        : mudlib_root_(mudlib_root) {}

    /**
     * @brief Get the collection of all objects in the sandbox.
     *
     * All objects in the sandbox can be found with an unique name.
     * If the object name contains a `#` character, it is a cloned object.
     * Cloned objects are typically created via the `clone_object` method.
     * Otherwise, the object is a blueprint object that can be cloned to
     * create new instances.
     *
     * One an object is loaded or cloned, the sandbox maintains a reference
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
    ObjectHandle find_object (const PathType &path);
    ObjectHandle clone_object (ObjectHandle obj);
    void destruct_object (ObjectHandle obj);

    friend class ::LpcObject;

private:
    PathType mudlib_root_;
    ObjectTable named_collection_; // named references to LpcObject instances

    ObjectTable& objects() { return named_collection_; }
};

} // namespace lpcvm
