// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>
#include "ast_node.hpp"
#include "lpc_compiler.hpp"

static_assert(!std::is_copy_constructible_v<ASTNode>);
static_assert(!std::is_copy_assignable_v<ASTNode>);
static_assert(std::is_move_constructible_v<ASTNode>);
static_assert(std::is_move_assignable_v<ASTNode>);

TEST(LpcCompilerTest, NonemptyDeclarationPrefixes) {
    for (const char* source : {
        "int x = 0, y = 0, z;",
        "auto x;",
        "private static x;",
        "private static int x;",
        "int f();",
        "private int *f();",
        "public f();",
        "create() {}",
        "*f();",
        "inherit \"base\"; private inherit \"other\";",
        "nomask void f() { private static x; private int y; int z; x = 1; x; }",
    }) {
        SCOPED_TRACE(source);
        LpcCompiler compiler;
        EXPECT_TRUE(compiler.frontend(source));
    }
}

TEST(LpcCompilerTest, RejectMissingOrRepeatedDeclarationTypes) {
    for (const char* source : {
        "x;",
        "x = 0;",
        "*x;",
        "int string x;",
        "float static x;",
        "private int string x;",
        "void f() { int string x; }",
    }) {
        SCOPED_TRACE(source);
        LpcCompiler compiler;
        EXPECT_FALSE(compiler.frontend(source));
    }
}

TEST(LpcCompilerTest, ErrorsIncludeCurrentSourceLine) {
    LpcCompiler compiler;
    const auto expect_error_line = [&compiler](const char* source, std::size_t line) {
        SCOPED_TRACE(source);
        EXPECT_FALSE(compiler.frontend(source));
        EXPECT_EQ (compiler.messages().back().lineno, line);
    };

    expect_error_line ("/* comment\nmore */\nint string x;", 3);
    expect_error_line ("\n\n@", 3);
    expect_error_line ("/* unfinished\n\n", 3);
    expect_error_line ("int x\n", 2);
    expect_error_line ("int string x;", 1);
}
