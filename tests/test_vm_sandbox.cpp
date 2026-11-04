// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_sandbox.hpp"
#include <gtest/gtest.h>
#include <cerrno>
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

TEST_F(SandboxFileTest, WritesAppendsAndUpdatesExistingFiles) {
    Sandbox sandbox(root);
    std::ofstream(root / "std/new.txt") << "old contents";
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
    EXPECT_FALSE(open(sandbox, "inside/new.txt", "w"));
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

TEST_F(SandboxFileTest, RequiresExistingFilesForEveryMode) {
    Sandbox sandbox(root);
    for (const char* mode : {"r", "w", "a", "r+", "w+", "a+"}) {
        errno = 0;
        EXPECT_FALSE(open(sandbox, "std/missing", mode)) << mode;
        EXPECT_EQ(errno, ENOENT);
        EXPECT_FALSE(std::filesystem::exists(root / "std/missing"));
    }
}

TEST_F(SandboxFileTest, FindsObjectsOnlyByExactName) {
    Sandbox sandbox(root);
    auto object = sandbox.load_object("std/player.lpc");
    ASSERT_TRUE(object);
    std::filesystem::create_directory_symlink(root / "std", root / "inside");
    EXPECT_EQ(sandbox.find_object("std/player").lock(), object.lock());
    for (const char* name : {"/std/player", "std/player.lpc", "./std/player",
                             "std/../std/player", "inside/player", ""}) {
        EXPECT_FALSE(sandbox.find_object(name)) << name;
    }
    auto clone = sandbox.clone_object(object);
    auto clone_ref = clone.lock();
    ASSERT_TRUE(clone_ref);
    const auto clone_name = std::string((*clone_ref)->name());
    EXPECT_EQ(sandbox.find_object(clone_name).lock(), clone.lock());
    EXPECT_FALSE(sandbox.find_object("/" + clone_name));

    // Neither successful lookups nor misses depend on the filesystem.
    std::filesystem::remove_all(root);
    EXPECT_EQ(sandbox.find_object("std/player").lock(), object.lock());
    EXPECT_EQ(sandbox.find_object(clone_name).lock(), clone.lock());
    EXPECT_FALSE(sandbox.find_object("missing"));
    EXPECT_FALSE(sandbox.find_object("../outside"));
}

TEST_F(SandboxFileTest, FindsObjectUsingNonNullTerminatedStringView) {
    Sandbox sandbox(root);
    auto object = sandbox.load_object("std/player.lpc");
    ASSERT_TRUE(object);
    const std::string name = "std/player.lpc-extra";
    EXPECT_EQ(sandbox.find_object(std::string_view(name.data(), 10)).lock(), object.lock());
    EXPECT_FALSE(sandbox.find_object(std::string_view(name.data(), 14)));
}

TEST_F(SandboxFileTest, ObjectKeysAreRelativeToEachMudlibRoot) {
    const auto other_root = temporary / "other-root";
    std::filesystem::create_directories(other_root / "std");
    std::ofstream(other_root / "std/player.lpc") << "int y;\n";
    // A symlink root must also be stripped from the object name.
    std::filesystem::create_directory_symlink(root, temporary / "root-link");
    for (const auto& mudlib : {root, other_root, temporary / "root-link"}) {
        Sandbox sandbox(mudlib);
        auto object = sandbox.load_object("/std/../std/player.lpc");
        auto object_ref = object.lock();
        ASSERT_TRUE(object_ref);
        EXPECT_EQ((*object_ref)->name(), "std/player");
        ASSERT_EQ(static_cast<const Sandbox&>(sandbox).objects().size(), 1u);
        EXPECT_EQ(static_cast<const Sandbox&>(sandbox).objects().begin()->first, "std/player");
        EXPECT_EQ(sandbox.find_object((*object_ref)->name()).lock(), object.lock());
        sandbox.destruct_object(object);
        EXPECT_TRUE(static_cast<const Sandbox&>(sandbox).objects().empty());
    }
}

TEST_F(SandboxFileTest, StripsSourceExtensionAndPreservesCloneSuffix) {
    Sandbox sandbox(root);
    std::filesystem::create_directory(root / "dir.with.dots");
    for (const char* filename : {"player.lpc", "player.test.lpc", "noextension"}) {
        const auto source = PathType("dir.with.dots") / filename;
        std::ofstream(root / source) << "int x;\n";
        auto key = source;
        key.replace_extension();
        auto object = sandbox.load_object(source);
        auto object_ref = object.lock();
        ASSERT_TRUE(object_ref);
        EXPECT_EQ((*object_ref)->name(), key.generic_string());
        EXPECT_EQ(sandbox.find_object(key.generic_string()).lock(), object.lock());
        auto clone = sandbox.clone_object(object);
        auto clone_ref = clone.lock();
        ASSERT_TRUE(clone_ref);
        const auto clone_name = std::string((*clone_ref)->name());
        ASSERT_EQ(clone_name.find(key.generic_string() + "#"), 0u);
        EXPECT_EQ(sandbox.find_object(clone_name).lock(), clone.lock());
        std::filesystem::remove(root / source);
        EXPECT_EQ(sandbox.find_object(key.generic_string()).lock(), object.lock());
        sandbox.destruct_object(clone);
        EXPECT_FALSE(sandbox.find_object(clone_name));
    }
}

TEST_F(SandboxFileTest, UnknownObjectNamesReturnEmptyHandles) {
    Sandbox sandbox(root);
    std::filesystem::create_directory_symlink(temporary / "mudlib-other", root / "outside");
    for (const char* path : {"../mudlib-other/secret", "../mudlib-other/missing/child",
                             "missing/../../mudlib-other/secret", "outside/secret",
                             "outside/missing/child"}) {
        EXPECT_FALSE(sandbox.find_object(path)) << path;
    }
    errno = 0;
    EXPECT_FALSE(open(sandbox, "outside/secret", "r"));
    EXPECT_EQ(errno, EACCES);
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
