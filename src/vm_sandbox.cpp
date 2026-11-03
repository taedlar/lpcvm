// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_exception.hpp"
#include "vm_sandbox.hpp"
#include "lpc_compiler.hpp"
#include <cerrno>

using namespace lpcvm;

FILE* Sandbox::file_open(const char* path, const char* mode) {
    if (!path || !*path || !mode || !*mode) {
        errno = EINVAL;
        return nullptr;
    }

    std::error_code error;
    const auto root = std::filesystem::canonical(mudlib_root_, error);
    if (error) {
        errno = error.value();
        return nullptr;
    }

    // LPC absolute paths refer to the mudlib, never the host filesystem root.
    const auto requested = root / PathType(path).relative_path();
    auto resolved = std::filesystem::canonical(requested, error);
    if (error == std::errc::no_such_file_or_directory) {
        // fopen may create the final component. Resolve its parent, but do not
        // treat a dangling symlink as a new file: fopen would follow that link.
        const auto status = std::filesystem::symlink_status(requested, error);
        if (status.type() == std::filesystem::file_type::not_found) {
            const auto parent = std::filesystem::canonical(requested.parent_path(), error);
            if (!error) resolved = parent / requested.filename();
        } else if (!error) {
            error = std::make_error_code(std::errc::no_such_file_or_directory);
        }
    }
    if (error) {
        errno = error.value();
        return nullptr;
    }

    // Compare path components, so a sibling named "mudlib-other" is excluded.
    auto candidate = resolved.begin();
    for (const auto& component : root) {
        if (candidate == resolved.end() || *candidate != component) {
            errno = EACCES;
            return nullptr;
        }
        ++candidate;
    }
    return std::fopen(resolved.string().c_str(), mode);
}

ObjectHandle Sandbox::load_object (const PathType &path) {
    FILE* file = file_open(path.string().c_str(), "r");
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    // Compile the object using the LPC compiler
    LpcCompiler compiler(file);
    if (!compiler.parse()) {
        fclose(file);
        return LpcObject::HandleType();
    }

    // Create and return the LPC object handle
    auto prog = compiler.generate_code();
    fclose(file);
    return LpcObject::create(this, path.string(), prog);
}

ObjectHandle Sandbox::find_object (const PathType &path) {
    auto obj_ref = LpcObject::find(this, path.string());
    return obj_ref;
}

ObjectHandle Sandbox::clone_object (ObjectHandle obj) {
    static uint32_t clone_counter = 0;
    auto obj_ref = obj.lock();
    if (obj_ref == std::nullopt) {
        return LpcObject::HandleType();
    }
    std::string clone_name = fmt::format("{}#{}", (*obj_ref)->name(), ++clone_counter);
    return LpcObject::create(this, clone_name, (*obj_ref)->program());
}

void Sandbox::destruct_object (ObjectHandle obj) {
    auto obj_ref = obj.lock();
    if (obj_ref != std::nullopt) {
        LpcObject::destruct(this, *obj_ref);
    }
}
