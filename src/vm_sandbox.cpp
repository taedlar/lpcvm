// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <fstream>
#include "vm_sandbox.hpp"
#include "lpc_compiler.hpp"

using namespace lpcvm;

LpcObject::HandleType Sandbox::load_object (const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    // Read the file content into a string
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    // Compile the object using the LPC compiler
    LpcCompiler compiler;
    if (!compiler.frontend(source))
        return LpcObject::HandleType();

    // Create and return the LPC object handle
    auto rt_class = compiler.backend();
    return LpcObject::create(path.string(), rt_class);
}

LpcObject::HandleType Sandbox::find_object (const std::filesystem::path &path) {
    auto obj_ref = LpcObject::find(path.string());
    return obj_ref;
}

LpcObject::HandleType Sandbox::clone_object (LpcObject::HandleType obj) {
    auto obj_ref = obj.lock();
    return obj_ref == std::nullopt ? LpcObject::HandleType() : LpcObject::HandleType(*obj_ref);
}
