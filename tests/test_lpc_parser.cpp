// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "ast_node.hpp"
#include "lpc_compiler.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<ASTNode>);
static_assert(!std::is_copy_assignable_v<ASTNode>);
static_assert(std::is_move_constructible_v<ASTNode>);
static_assert(std::is_move_assignable_v<ASTNode>);
static_assert(std::is_assignable_v<ASTNode&, ASTNode&>);

TEST(ASTNodeTest, MutableAssignmentTransfersOwnership) {
    ASTNode source{AST::Return{std::make_unique<ASTNode>(AST::NumericConst{42L})}, {3, 5}};
    auto* child = std::get<AST::Return>(source.node).expr.get();
    ASTNode destination{AST::Block{}};

    EXPECT_EQ(&(destination = source), &destination);
    EXPECT_EQ(std::get<AST::Return>(destination.node).expr.get(), child);
    EXPECT_EQ(std::get<AST::Return>(source.node).expr, nullptr);
    EXPECT_EQ(destination.src.lineno, 3);
    EXPECT_EQ(destination.src.column, 5);
}

TEST(LpcParserTest, ImplicitBlockAction) {
    LpcCompiler compiler;
    EXPECT_TRUE(compiler.parse("void f() { { return 42; } }"));
    ASSERT_NE(compiler.get_ast(), nullptr);
    auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    ASSERT_EQ(defs.size(), 1);
    auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
    auto& stmts = std::get<AST::Block>(body.node).stmts;

    ASSERT_TRUE(std::holds_alternative<AST::List>(stmts->node));
    auto& stmts_list = std::get<AST::List>(stmts->node).elts;
    ASSERT_TRUE(std::holds_alternative<AST::Block>(stmts_list.front().node)); // first statement in f() is a nested block
    auto& nested = std::get<AST::Block>(stmts_list.front().node).stmts;
    ASSERT_TRUE(std::holds_alternative<AST::List>(nested->node));
    auto& nested_list = std::get<AST::List>(nested->node).elts;
    ASSERT_EQ(nested_list.size(), 1);
    EXPECT_TRUE(std::holds_alternative<AST::Return>(nested_list.front().node));
}

TEST(LpcParserTest, NestedBlocksCountOwnLocals) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse("void f(int p) { int a = 1; { int x = 2; } int y = 3; }"));
    ASSERT_NE(compiler.get_ast(), nullptr);
    auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    ASSERT_EQ(defs.size(), 1);

    auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
    auto& body_block = std::get<AST::Block>(body.node);
    EXPECT_EQ(body_block.num_locals, 3);

    auto& statements = std::get<AST::List>(body_block.stmts->node).elts;
    ASSERT_EQ(statements.size(), 3);
    auto& nested_block = std::get<AST::Block>(statements[1].node);
    EXPECT_EQ(nested_block.num_locals, 1);
}

TEST(LpcParserTest, AstNodesHaveSourceLocations) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse("/* heading */\nvoid f(int p) {\n  return p + 42;\n}\n"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    ASSERT_EQ(defs.size(), 1);
    const auto expect_location = [](const ASTNode& node, int line, int column) {
        EXPECT_EQ(node.src.lineno, line);
        EXPECT_EQ(node.src.column, column);
    };
    expect_location(defs.front(), 2, 1);
    const auto& function = std::get<AST::FunctionDef>(defs.front().node);
    expect_location(*function.decl, 2, 1);
    expect_location(*std::get<AST::FunctionDecl>(function.decl->node).param_list, 2, 8);
    expect_location(*function.body, 2, 15);
    const auto& statements = std::get<AST::List>(std::get<AST::Block>(function.body->node).stmts->node).elts;
    ASSERT_EQ(statements.size(), 1);
    expect_location(statements.front(), 3, 3);
    const auto& expression = *std::get<AST::Return>(statements.front().node).expr;
    expect_location(expression, 3, 10);
    const auto& addition = std::get<AST::BinaryOp>(expression.node);
    expect_location(*addition.lhs, 3, 10);
    expect_location(*addition.rhs, 3, 14);
}

TEST(LpcParserTest, SynthesizedNodesHaveSourceLocations) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(
        "\ninherit \"base\";\nint prototype();\ncreate() {\n"
        "  for (int count = 0; count < 2; count++) {\n"
        "    if (count) continue; else break;\n"
        "  }\n  return \"first\" \"second\";\n}\n"));
    const auto expect_location = [](const ASTNode& node, int line, int column) {
        EXPECT_EQ(node.src.lineno, line);
        EXPECT_EQ(node.src.column, column);
    };
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    ASSERT_EQ(defs.size(), 3);
    expect_location(*compiler.get_ast(), 1, 1);
    expect_location(defs[0], 2, 1);
    expect_location(defs[1], 3, 1);
    expect_location(*std::get<AST::FunctionDecl>(defs[1].node).param_list, 3, 15);
    expect_location(defs[2], 4, 1);
    const auto& function = std::get<AST::FunctionDef>(defs[2].node);
    expect_location(*function.decl, 4, 1);
    expect_location(*function.body, 4, 10);
    const auto& body = std::get<AST::Block>(function.body->node);
    expect_location(*body.stmts, 5, 3);
    const auto& statements = std::get<AST::List>(body.stmts->node).elts;
    ASSERT_EQ(statements.size(), 2);
    expect_location(statements[0], 5, 3);
    const auto& loop_block = std::get<AST::Block>(statements[0].node);
    expect_location(*loop_block.stmts, 5, 12);
    const auto& loop_statements = std::get<AST::List>(loop_block.stmts->node).elts;
    ASSERT_EQ(loop_statements.size(), 2);
    expect_location(loop_statements[0], 5, 12);
    expect_location(*std::get<AST::BinaryOp>(loop_statements[0].node).lhs, 5, 12);
    expect_location(loop_statements[1], 5, 3);
    const auto& loop = std::get<AST::PreTestedLoop>(loop_statements[1].node);
    expect_location(*loop.condition, 5, 23);
    expect_location(*loop.step, 5, 34);
    expect_location(*loop.body, 5, 43);
    const auto& loop_body = std::get<AST::Block>(loop.body->node);
    const auto& condition_node = std::get<AST::List>(loop_body.stmts->node).elts.front();
    expect_location(condition_node, 6, 5);
    const auto& condition = std::get<AST::Cond>(condition_node.node);
    expect_location(*condition.body, 6, 16);
    expect_location(*condition.else_body, 6, 31);
    expect_location(statements[1], 8, 3);
    expect_location(*std::get<AST::Return>(statements[1].node).expr, 8, 10);

    ASSERT_TRUE(compiler.parse("void empty() {}"));
    const auto& empty_function = std::get<AST::FunctionDef>(
        std::get<AST::Program>(compiler.get_ast()->node).defs.front().node);
    expect_location(*empty_function.decl, 1, 1);
    expect_location(*std::get<AST::Block>(empty_function.body->node).stmts, 1, 15);
}

TEST(LpcParserTest, ErrorsIncludeTokenColumn) {
    LpcCompiler compiler;
    ASSERT_FALSE(compiler.parse("\n  int string x;"));
    EXPECT_EQ(compiler.messages().back().lineno, 2);
    EXPECT_EQ(compiler.messages().back().column, 7);
    ASSERT_FALSE(compiler.parse("\n  @"));
    EXPECT_EQ(compiler.messages().back().lineno, 2);
    EXPECT_EQ(compiler.messages().back().column, 3);
    ASSERT_FALSE(compiler.parse("int x\n  "));
    EXPECT_EQ(compiler.messages().back().lineno, 2);
    EXPECT_EQ(compiler.messages().back().column, 3);
}

TEST(LpcParserTest, NonemptyDeclarationPrefixes) {
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
        EXPECT_TRUE(compiler.parse(source));
    }
}

TEST(LpcParserTest, RejectMissingOrRepeatedDeclarationTypes) {
    LpcCompiler compiler;
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
        EXPECT_FALSE(compiler.parse(source));
    }
}

TEST(LpcParserTest, ErrorsIncludeCurrentSourceLine) {
    LpcCompiler compiler;
    const auto expect_error_line = [&compiler](const char* source, std::size_t line) {
        SCOPED_TRACE(source);
        EXPECT_FALSE(compiler.parse(source));
        EXPECT_EQ (compiler.messages().back().lineno, line);
    };

    expect_error_line ("/* comment\nmore */\nint string x;", 3);
    expect_error_line ("\n\n@", 3);
    expect_error_line ("/* unfinished\n\n", 3);
    expect_error_line ("int x\n", 2);
    expect_error_line ("int string x;", 1);
}
