// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <filesystem>
#include "lpc_types.hpp"

namespace lpcvm {

class Sandbox {
public:
    Sandbox(const std::filesystem::path &mudlib_root)
        : mudlib_root_(mudlib_root) {}

    LpcObject::HandleType load_object (const std::filesystem::path &path);
    LpcObject::HandleType find_object (const std::filesystem::path &path);
    LpcObject::HandleType clone_object (LpcObject::HandleType obj);
    void destruct_object (LpcObject::HandleType obj);

private:
    std::filesystem::path mudlib_root_;
};

} // namespace lpcvm
