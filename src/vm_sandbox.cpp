// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <cstdio>
#include "vm_sandbox.hpp"
#include "lpc_compiler.hpp"

using namespace lpcvm;

ObjectHandle Sandbox::load_object (const PathType &path) {
    FILE* file = fopen(path.string().c_str(), "r");
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
