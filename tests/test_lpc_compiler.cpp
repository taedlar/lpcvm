// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include <gtest/gtest.h>
#include <stdexcept>
#include "lpc_compiler.hpp"

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
        EXPECT_NO_THROW(compiler.compile(source));
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
        EXPECT_THROW(compiler.compile(source), std::runtime_error);
    }
}
