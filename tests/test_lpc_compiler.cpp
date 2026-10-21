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
static_assert(std::is_assignable_v<ASTNode&, ASTNode&>);

TEST(ASTNodeTest, MutableAssignmentTransfersOwnership) {
    ASTNode source{AST::Return{std::make_unique<ASTNode>(AST::Constant{42L})}};
    auto* child = std::get<AST::Return>(source.node).value.get();
    ASTNode destination{AST::Block{}};

    EXPECT_EQ(&(destination = source), &destination);
    EXPECT_EQ(std::get<AST::Return>(destination.node).value.get(), child);
    EXPECT_EQ(std::get<AST::Return>(source.node).value, nullptr);
}

TEST(LpcCompilerTest, ImplicitBlockAction) {
    LpcCompiler compiler;
    auto root = compiler.frontend("void f() { { return 42; } }");
    ASSERT_NE(root.get(), nullptr);
    auto& defs = std::get<AST::Program>(root->node).defs;
    ASSERT_EQ(defs.size(), 1);
    auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
    auto& stmts = std::get<AST::Block>(body.node).stmts;
    ASSERT_EQ(stmts.size(), 1);
    auto& nested = std::get<AST::Block>(stmts.front().node).stmts;
    ASSERT_EQ(nested.size(), 1);
    EXPECT_TRUE(std::holds_alternative<AST::Return>(nested.front().node));
}

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
