// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_exception.hpp"
#include "vm_sandbox.hpp"
#include "lpc_compiler.hpp"
#include "vm_file_utils.hpp"
#include <utility>
#include <cerrno>

using namespace lpcvm;

// Clone suffixes belong to the object name, not the source extension.
static std::string object_key(PathType local_path) {
    auto filename = local_path.filename().generic_string();
    const auto clone = filename.find('#');
    const auto suffix = clone == std::string::npos ? std::string() : filename.substr(clone);
    PathType source_name(filename.substr(0, clone));
    source_name.replace_extension();
    local_path.replace_filename(source_name.generic_string() + suffix);
    return local_path.generic_string();
}

FILE* Sandbox::file_open(const char* path, const char* mode) {
    using namespace lpcvm::detail;
    if (!path || !*path || !mode || !*mode) {
        errno = EINVAL;
        return nullptr;
    }

    try {
        const auto resolved = canonicalize_object_path(path, mudlib_root_);
        return std::fopen(resolved.filesystem_path.string().c_str(), mode);
    } catch (const fs::filesystem_error& error) {
        errno = error.code().value();
        return nullptr;
    }
}

std::optional<PathType> Sandbox::file_search(
    const PathType& name, const std::vector<PathType>& search_paths) {
    using namespace lpcvm::detail;
    return search_file(name, search_paths, [this](const auto& candidate) {
        const auto resolved = canonicalize_object_path(candidate, mudlib_root_);
        return std::make_pair(resolved.filesystem_path, resolved.local_path);
    });
}

ObjectHandle Sandbox::load_object (const PathType &path) {
    using namespace lpcvm::detail;
    const auto canonical_path = canonicalize_object_path(path, mudlib_root_);
    FILE* file = std::fopen(canonical_path.filesystem_path.string().c_str(), "r");
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    // Compile the object using the LPC compiler
    LpcCompiler compiler(file);
    // Compilation is synchronous; the compiler borrows this sandbox's fabric.
    compiler.use_fabric(std::shared_ptr<ManagedRuntime>(this, [](ManagedRuntime*) {}));
    compiler.set_filename(canonical_path.local_path.generic_string());
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

ObjectHandle Sandbox::clone_object (ObjectHandle obj) {
    static uint32_t clone_counter = 0;
    auto obj_ref = obj.lock();
    if (obj_ref == std::nullopt) {
        return LpcObject::HandleType();
    }
    const auto name = (*obj_ref)->name();
    std::string clone_name = fmt::format("{}#{}", name ? name->view() : std::string_view{}, ++clone_counter);
    return LpcObject::create(this, clone_name, (*obj_ref)->program());
}
