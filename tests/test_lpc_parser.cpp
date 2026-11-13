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
    ASTNode source{AST::Return{std::make_unique<ASTNode>(AST::NumericConst{42L})}, {0, 0, 3, 5}};
    auto* child = std::get<AST::Return>(source.node).expr.get();
    ASTNode destination{AST::Block{}};

    EXPECT_EQ(&(destination = source), &destination);
    EXPECT_EQ(std::get<AST::Return>(destination.node).expr.get(), child);
    EXPECT_EQ(std::get<AST::Return>(source.node).expr, nullptr);
    EXPECT_EQ(destination.src.current_file, 0);
    EXPECT_EQ(destination.src.current_func, 0);
    EXPECT_EQ(destination.src.lineno, 3);
    EXPECT_EQ(destination.src.column, 5);
}

TEST(LpcParserTest, AssignmentAssociativityAndPrecedence) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(
        "int f(int a, int b, int c) { return a = b += c + 2 * 3; }"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
    const auto& statements = std::get<AST::List>(std::get<AST::Block>(body.node).stmts->node).elts;
    const auto& result = *std::get<AST::Return>(statements.front().node).expr;
    const auto& assignment = std::get<AST::Assign>(result.node);
    EXPECT_EQ(assignment.arg, oparg::ASSIGN);
    EXPECT_EQ(std::get<AST::Variable>(assignment.lhs->node).entry->name, "a");
    const auto& compound = std::get<AST::Assign>(assignment.rhs->node);
    EXPECT_EQ(compound.arg, oparg::ASSIGN_ADD);
    EXPECT_EQ(std::get<AST::Variable>(compound.lhs->node).entry->name, "b");
    const auto& addition = std::get<AST::BinaryOp>(compound.rhs->node);
    EXPECT_EQ(addition.arg, oparg::ADD);
    EXPECT_EQ(std::get<AST::BinaryOp>(addition.rhs->node).arg, oparg::MUL);
}

TEST(LpcParserTest, IncrementAndDecrementInExpressions) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(
        "int f(int a, int b) { a = ++b + a--; return --a * b++; }"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
    const auto& statements = std::get<AST::List>(std::get<AST::Block>(body.node).stmts->node).elts;
    ASSERT_EQ(statements.size(), 2);
    const auto& assignment = std::get<AST::Assign>(statements[0].node);
    const auto& addition = std::get<AST::BinaryOp>(assignment.rhs->node);
    EXPECT_EQ(addition.arg, oparg::ADD);
    EXPECT_EQ(std::get<AST::Assign>(addition.lhs->node).arg, oparg::ASSIGN_PREFIX_INC);
    EXPECT_EQ(std::get<AST::Assign>(addition.rhs->node).arg, oparg::ASSIGN_SUFFIX_DEC);
    const auto& result = *std::get<AST::Return>(statements[1].node).expr;
    const auto& product = std::get<AST::BinaryOp>(result.node);
    EXPECT_EQ(product.arg, oparg::MUL);
    EXPECT_EQ(std::get<AST::Assign>(product.lhs->node).arg, oparg::ASSIGN_PREFIX_DEC);
    EXPECT_EQ(std::get<AST::Assign>(product.rhs->node).arg, oparg::ASSIGN_SUFFIX_INC);
}

TEST(LpcParserTest, SwitchLabels) {
    for (const auto* source : {
        "void f(int x) { switch (x) { case 1: break; default: return; } }",
        "void f(int x) { switch (x) { case 1: case 2: default: break; } }",
        "void f(string x) { switch (x) { case \"one\": return; } }",
        "void f(int x) { switch (x) { case 1: switch (x) { case 2: break; } break; } }",
        "void f(int x) { switch (x) { case 1: switch (x) { default: break; } case 2: break; } }",
        "void f(int x) { switch (x) { { case 1: break; } default: return; } }",
        "void f(int x) { switch (x) { return; } return; }"
    }) {
        SCOPED_TRACE(source);
        LpcCompiler compiler;
        EXPECT_TRUE(compiler.parse(source));
    }
}

TEST(LpcParserTest, RejectsMalformedSwitchLabels) {
    for (const auto* source : {
        "void f(int x) { switch (x) { case : break; } }",
        "void f(int x) { switch (x) { case 1 break; } }",
        "void f(int x) { switch (x) { default break; } }"
    }) {
        SCOPED_TRACE(source);
        LpcCompiler compiler;
        EXPECT_FALSE(compiler.parse(source));
    }
}

TEST(LpcParserTest, RejectsLabelsOutsideSwitch) {
    for (const auto* source : {
        "void f() { case 1: return; }",
        "void f() { default: return; }",
        "void f() { { case 1: return; } }",
        "void f(int x) { while (x) { default: break; } }",
        "void f(int x) { switch (x) { case 1: break; } case 2: return; }",
        "void f(int x) { switch (x) { default: break; } default: return; }",
        "void f(int x) { switch (x) { default: break; } } void g() { case 1: return; }"
    }) {
        SCOPED_TRACE(source);
        LpcCompiler compiler;
        ASSERT_FALSE(compiler.parse(source));
        ASSERT_FALSE(compiler.messages().empty());
        EXPECT_NE(compiler.messages().back().text.find("label outside switch"), std::string::npos);
    }
}

TEST(LpcParserTest, SwitchLabelErrorLocationAndParserReuse) {
    LpcCompiler compiler;
    ASSERT_FALSE(compiler.parse("void f() {\n  case 1: return;\n}"));
    EXPECT_EQ(compiler.messages().back().lineno, 2);
    EXPECT_EQ(compiler.messages().back().column, 3);
    ASSERT_FALSE(compiler.parse("void f(int x) { switch (x) { case : break; } }"));
    EXPECT_FALSE(compiler.parse("void f() { default: return; }"));
    EXPECT_TRUE(compiler.parse("void f(int x) { switch (x) { default: return; } }"));
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
    expect_location(*std::get<AST::Assign>(loop_statements[0].node).lhs, 5, 12);
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

TEST(LpcParserTest, ErrorsIncludeFilename) {
    LpcCompiler compiler;
    compiler.set_filename("example.lpc");
    for (const auto& source : {"\n  int string x;", "\n  @", "void f() { return missing; }"}) {
        SCOPED_TRACE(source);
        ASSERT_FALSE(compiler.parse(source));
        ASSERT_FALSE(compiler.messages().empty());
        const auto& message = compiler.messages().back();
        const auto prefix = "example.lpc:" + std::to_string(message.lineno)
            + ":" + std::to_string(message.column) + ": ";
        EXPECT_EQ(message.text.find(prefix), 0);
    }

    compiler.set_filename("other.lpc");
    ASSERT_FALSE(compiler.parse("\n  @"));
    EXPECT_EQ(compiler.messages().back().text, "other.lpc:2:3: Unexpected character");

    compiler.set_filename("");
    ASSERT_FALSE(compiler.parse("\n  @"));
    EXPECT_EQ(compiler.messages().back().text, "Unexpected character");
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

TEST(LpcParserTest, OptionalFunctionCallPrefixes) {
    for (const auto* prefix : {"", ".", "->", "::"}) {
        SCOPED_TRACE(prefix);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(std::string("int f(); void g(int x) { ")
            + prefix + "f(x, key = 2); }"));
        const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
        const auto& body = *std::get<AST::FunctionDef>(defs.back().node).body;
        const auto& statements = std::get<AST::List>(std::get<AST::Block>(body.node).stmts->node).elts;
        ASSERT_EQ(statements.size(), 1);
        const auto& call = std::get<AST::FunctionCall>(statements.front().node);
        ASSERT_NE(call.entry, nullptr);
        EXPECT_EQ(call.entry->name, "f");
        const auto& arguments = std::get<AST::List>(call.arguments->node).elts;
        ASSERT_EQ(arguments.size(), 2);
        EXPECT_EQ(std::get<AST::Variable>(arguments[0].node).entry->name, "x");
        EXPECT_TRUE(std::holds_alternative<AST::KeywordArgument>(arguments[1].node));
    }
}

TEST(LpcParserTest, PositionalAndKeywordArguments) {
    for (const auto* arguments : {
        "", "1", "1, 2", "key = 1", "key = 1, other = 2",
        "1, key = 2", "1, 2, key = 3, other = 4",
        "(x = 1), key = x + 2", "x += 1, key = 2",
        "f(1, key = 2), other = f(key = 3)"
    }) {
        SCOPED_TRACE(arguments);
        LpcCompiler compiler;
        EXPECT_TRUE(compiler.parse(std::string("int f(); void g(int x) { f(") + arguments + "); }"));
    }
}

TEST(LpcParserTest, RejectsMalformedArgumentLists) {
    for (const auto* arguments : {
        ", 1", ", key = 1", "1,", "key = 1,", "1,,2",
        "key = 1, 2", "1, key = 2, 3"
    }) {
        SCOPED_TRACE(arguments);
        LpcCompiler compiler;
        EXPECT_FALSE(compiler.parse(std::string("int f(); void g() { f(") + arguments + "); }"));
    }
}

TEST(LpcParserTest, ArgumentListsPreserveKeywordValuesAndPositionalAssignments) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(
        "int f(); void g(int x) { f((x = 1), key = x + 2, other = 3); f(key = 4); }"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& body = *std::get<AST::FunctionDef>(defs.back().node).body;
    const auto& statements = std::get<AST::List>(std::get<AST::Block>(body.node).stmts->node).elts;
    const auto& call = std::get<AST::FunctionCall>(statements[0].node);
    const auto& args = std::get<AST::List>(call.arguments->node).elts;
    ASSERT_EQ(args.size(), 3);
    EXPECT_EQ(std::get<AST::Assign>(args[0].node).arg, oparg::ASSIGN);
    const auto& keyword = std::get<AST::KeywordArgument>(args[1].node);
    ASSERT_NE(keyword.value, nullptr);
    EXPECT_EQ(std::get<AST::BinaryOp>(keyword.value->node).arg, oparg::ADD);
    EXPECT_TRUE(std::holds_alternative<AST::KeywordArgument>(args[2].node));
    const auto& keyword_only = std::get<AST::FunctionCall>(statements[1].node);
    const auto& keywords = std::get<AST::List>(keyword_only.arguments->node).elts;
    ASSERT_EQ(keywords.size(), 1);
    EXPECT_TRUE(std::holds_alternative<AST::KeywordArgument>(keywords[0].node));
}

TEST(LpcParserTest, AggregateListsPreserveElementOrder) {
    for (const auto* expression : {"({a, b, c, d})", "([a:b, c:d])"}) {
        SCOPED_TRACE(expression);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(
            std::string("mixed f(int a, int b, int c, int d) { return ") + expression + "; }"));
        const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
        const auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
        const auto& statements = std::get<AST::List>(std::get<AST::Block>(body.node).stmts->node).elts;
        const auto& result = *std::get<AST::Return>(statements.front().node).expr;
        const auto& elements = std::get<AST::List>(result.node).elts;
        ASSERT_EQ(elements.size(), 4);
        for (size_t i = 0; i < elements.size(); ++i) {
            EXPECT_EQ(std::get<AST::Variable>(elements[i].node).entry->name,
                      std::string(1, static_cast<char>('a' + i)));
            EXPECT_EQ(elements[i].src.lineno, 1);
            EXPECT_GT(elements[i].src.column, result.src.column);
        }
    }
}

TEST(LpcParserTest, AggregateListsPreserveNestedExpressions) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse("mixed f(int a, int b) { return ({([a:b + 1])}); }"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
    const auto& statements = std::get<AST::List>(std::get<AST::Block>(body.node).stmts->node).elts;
    const auto& result = *std::get<AST::Return>(statements.front().node).expr;
    const auto& elements = std::get<AST::List>(result.node).elts;
    ASSERT_EQ(elements.size(), 1);
    const auto& pair = std::get<AST::List>(elements[0].node).elts;
    ASSERT_EQ(pair.size(), 2);
    EXPECT_EQ(std::get<AST::Variable>(pair[0].node).entry->name, "a");
    const auto& addition = std::get<AST::BinaryOp>(pair[1].node);
    EXPECT_EQ(addition.arg, oparg::ADD);
    ASSERT_NE(addition.lhs, nullptr);
    ASSERT_NE(addition.rhs, nullptr);
    EXPECT_EQ(std::get<AST::Variable>(addition.lhs->node).entry->name, "b");
    EXPECT_TRUE(std::holds_alternative<AST::NumericConst>(addition.rhs->node));
}
