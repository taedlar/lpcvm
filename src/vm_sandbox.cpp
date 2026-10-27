// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <cstdio>
#include "vm_sandbox.hpp"
#include "lpc_compiler.hpp"

using namespace lpcvm;

LpcObject::HandleType Sandbox::load_object (const std::filesystem::path &path) {
    FILE* file = fopen(path.c_str(), "r");
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
    return LpcObject::create(path.string(), prog);
}

LpcObject::HandleType Sandbox::find_object (const std::filesystem::path &path) {
    auto obj_ref = LpcObject::find(path.string());
    return obj_ref;
}

LpcObject::HandleType Sandbox::clone_object (LpcObject::HandleType obj) {
    static uint32_t clone_counter = 0;
    auto obj_ref = obj.lock();
    if (obj_ref == std::nullopt) {
        return LpcObject::HandleType();
    }
    std::string clone_name = fmt::format("{}#{}", (*obj_ref)->name(), ++clone_counter);
    return LpcObject::create(clone_name, (*obj_ref)->program());
}

void Sandbox::destruct_object (LpcObject::HandleType obj) {
    auto obj_ref = obj.lock();
    if (obj_ref != std::nullopt) {
        LpcObject::destruct(*obj_ref);
    }
}
