// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_exception.hpp"
#include "vm_sandbox.hpp"
#include "lpc_compiler.hpp"
#include <cerrno>

using namespace lpcvm;

namespace {
struct CanonicalObjectPath {
    PathType filesystem_path;
    PathType local_path;
};

// LPC absolute paths refer to the mudlib, never the host filesystem root.
CanonicalObjectPath canonicalize_object_path(const PathType &path, const PathType &mudlib_root) {
    const auto root = std::filesystem::canonical(mudlib_root);
    const auto requested = root / path.relative_path();
    const auto resolved = std::filesystem::canonical(requested);

    // Compare components so a sibling named "mudlib-other" is excluded.
    auto candidate = resolved.begin();
    for (const auto& component : root) {
        if (candidate == resolved.end() || *candidate != component) {
            throw std::filesystem::filesystem_error(
                "Access denied", resolved,
                std::make_error_code(std::errc::permission_denied));
        }
        ++candidate;
    }
    return {resolved, resolved.lexically_relative(root)};
}
// Clone suffixes belong to the object name, not the source extension.
std::string object_key(PathType local_path) {
    auto filename = local_path.filename().generic_string();
    const auto clone = filename.find('#');
    const auto suffix = clone == std::string::npos ? std::string() : filename.substr(clone);
    PathType source_name(filename.substr(0, clone));
    source_name.replace_extension();
    local_path.replace_filename(source_name.generic_string() + suffix);
    return local_path.generic_string();
}
} // namespace

FILE* Sandbox::file_open(const char* path, const char* mode) {
    if (!path || !*path || !mode || !*mode) {
        errno = EINVAL;
        return nullptr;
    }

    try {
        const auto resolved = canonicalize_object_path(path, mudlib_root_);
        return std::fopen(resolved.filesystem_path.string().c_str(), mode);
    } catch (const std::filesystem::filesystem_error& error) {
        errno = error.code().value();
        return nullptr;
    }
}

ObjectHandle Sandbox::load_object (const PathType &path) {
    const auto canonical_path = canonicalize_object_path(path, mudlib_root_);
    FILE* file = std::fopen(canonical_path.filesystem_path.string().c_str(), "r");
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
    if (!prog)
        return LpcObject::HandleType();

    return LpcObject::create(this, object_key(canonical_path.local_path), prog);
}

ObjectHandle Sandbox::find_object (std::string_view name) {
    auto obj_ref = LpcObject::find(this, name);
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
