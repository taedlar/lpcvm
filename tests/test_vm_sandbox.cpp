// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_sandbox.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>

using namespace lpcvm;

namespace {
class SandboxFileTest : public ::testing::Test {
protected:
    std::filesystem::path temporary;
    std::filesystem::path root;
    struct CloseFile {
        void operator()(FILE* file) const { std::fclose(file); }
    };
    using File = std::unique_ptr<FILE, CloseFile>;

    void SetUp() override {
        std::random_device random;
        do {
            temporary = std::filesystem::temp_directory_path() /
                ("lpcvm-sandbox-" + std::to_string(random()));
        } while (!std::filesystem::create_directory(temporary));
        root = temporary / "mudlib";
        std::filesystem::create_directories(root / "std");
        std::filesystem::create_directory(temporary / "mudlib-other");
        std::ofstream(root / "std/player.lpc") << "int x;\n";
        std::ofstream(temporary / "mudlib-other/secret") << "secret";
    }

    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(temporary, error);
    }

    File open(Sandbox& sandbox, const char* path, const char* mode) {
        return File(sandbox.file_open(path, mode));
    }
};
}

TEST_F(SandboxFileTest, ReadsMudlibRelativeAndAbsolutePaths) {
    Sandbox sandbox(root);
    for (const char* path : {"std/player.lpc", "/std/player.lpc",
                             "std/../std/player.lpc"}) {
        auto file = open(sandbox, path, "r");
        ASSERT_TRUE(file) << path;
        EXPECT_EQ(std::fgetc(file.get()), 'i');
    }
}

TEST_F(SandboxFileTest, CreatesAppendsAndUpdatesFiles) {
    Sandbox sandbox(root);
    {
        auto file = open(sandbox, "/std/new.txt", "w");
        ASSERT_TRUE(file);
        EXPECT_GE(std::fputs("abc", file.get()), 0);
    }
    {
        auto file = open(sandbox, "std/new.txt", "a");
        ASSERT_TRUE(file);
        EXPECT_GE(std::fputs("d", file.get()), 0);
    }
    {
        auto file = open(sandbox, "std/new.txt", "r+");
        ASSERT_TRUE(file);
        EXPECT_EQ(std::fgetc(file.get()), 'a');
        EXPECT_EQ(std::fseek(file.get(), 0, SEEK_SET), 0);
        EXPECT_NE(std::fputc('z', file.get()), EOF);
    }
    std::ifstream file(root / "std/new.txt");
    std::string contents;
    file >> contents;
    EXPECT_EQ(contents, "zbcd");
}

TEST_F(SandboxFileTest, RejectsTraversalForReadsAndWrites) {
    Sandbox sandbox(root);
    for (const char* path : {"../mudlib-other/secret", "/../mudlib-other/secret",
                             "std/../../mudlib-other/secret"}) {
        EXPECT_FALSE(open(sandbox, path, "r")) << path;
        EXPECT_FALSE(open(sandbox, path, "w")) << path;
    }
    EXPECT_FALSE(open(sandbox, "../mudlib-other/new.txt", "w"));
    EXPECT_FALSE(std::filesystem::exists(temporary / "mudlib-other/new.txt"));
    EXPECT_EQ(std::filesystem::file_size(temporary / "mudlib-other/secret"), 6);
}

TEST_F(SandboxFileTest, ChecksSymlinkTargetsIncludingNewFiles) {
    Sandbox sandbox(root);
    std::filesystem::create_directory_symlink(root / "std", root / "inside");
    std::filesystem::create_directory_symlink(temporary / "mudlib-other", root / "outside");
    std::filesystem::create_symlink(temporary / "mudlib-other/secret", root / "secret-link");
    std::filesystem::create_symlink(temporary / "mudlib-other/missing", root / "dangling");
    EXPECT_TRUE(open(sandbox, "inside/player.lpc", "r"));
    EXPECT_TRUE(open(sandbox, "inside/new.txt", "w"));
    EXPECT_FALSE(open(sandbox, "outside/secret", "r"));
    EXPECT_FALSE(open(sandbox, "outside/new.txt", "w"));
    EXPECT_FALSE(open(sandbox, "secret-link", "w"));
    EXPECT_FALSE(open(sandbox, "dangling", "w"));
    EXPECT_FALSE(std::filesystem::exists(temporary / "mudlib-other/missing"));
    EXPECT_FALSE(std::filesystem::exists(temporary / "mudlib-other/new.txt"));
    EXPECT_EQ(std::filesystem::file_size(temporary / "mudlib-other/secret"), 6);
}

TEST_F(SandboxFileTest, FailsForInvalidInputsAndMissingFiles) {
    Sandbox sandbox(root);
    EXPECT_FALSE(open(sandbox, nullptr, "r"));
    EXPECT_FALSE(open(sandbox, "", "r"));
    EXPECT_FALSE(open(sandbox, "std/player.lpc", nullptr));
    EXPECT_FALSE(open(sandbox, "std/player.lpc", ""));
    EXPECT_FALSE(open(sandbox, "missing", "r"));
    EXPECT_FALSE(open(sandbox, "missing/child", "w"));
    Sandbox missing(temporary / "missing");
    EXPECT_FALSE(open(missing, "file", "w"));
}

TEST_F(SandboxFileTest, LoadObjectUsesMudlibRoot) {
    Sandbox sandbox(root);
    EXPECT_TRUE(sandbox.load_object("/std/player.lpc"));
    EXPECT_THROW(sandbox.load_object("../mudlib-other/secret"), std::runtime_error);
}

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
