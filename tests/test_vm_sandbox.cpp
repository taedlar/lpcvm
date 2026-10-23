// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_sandbox.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace lpcvm;

TEST(VmSandboxTest, LoadObject) {
    Sandbox sandbox (std::filesystem::current_path());
    // create a LPC file for testing
    std::ofstream file("example.lpc");
    file << "int x;\n";
    file.close();
    auto obj = sandbox.load_object("example.lpc");
    EXPECT_TRUE(obj);
    std::filesystem::remove("example.lpc");
}
