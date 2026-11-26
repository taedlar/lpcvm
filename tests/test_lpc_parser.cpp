// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "ast_node.hpp"
#include "lpc_compiler.hpp"
#include "native_module.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<ASTNode>);
static_assert(!std::is_copy_assignable_v<ASTNode>);
static_assert(std::is_move_constructible_v<ASTNode>);
static_assert(std::is_move_assignable_v<ASTNode>);
static_assert(std::is_assignable_v<ASTNode&, ASTNode&>);

namespace {
class ParserTestModule : public NativeModule {
public:
    ParserTestModule(std::shared_ptr<ManagedRuntime> fabric, std::string name)
        : NativeModule(std::move(fabric)), name_(std::move(name)) {}
    std::string_view name() const override { return name_; }
private:
    std::string name_;
};

std::shared_ptr<ManagedRuntime> mount_test_fabric(std::initializer_list<const char*> names) {
    auto fabric = std::make_shared<ManagedRuntime>();
    for (const auto* name : names)
        fabric->emplace_object(lpcvm::create_shared<ParserTestModule>(fabric, name));
    return fabric;
}
}

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

TEST(LpcParserTest, CatchPreservesExpressionAndBlockASTs) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(
        "void f(int x) {\n"
        "  catch (x = 1, x + 2);\n"
        "  catch { int local; local = x; };\n"
        "  catch {};\n"
        "}"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& body = *std::get<AST::FunctionDef>(defs.front().node).body;
    const auto& statements = std::get<AST::List>(std::get<AST::Block>(body.node).stmts->node).elts;
    ASSERT_EQ(statements.size(), 3u);
    const auto& expression = std::get<AST::Catch>(statements[0].node).expr;
    ASSERT_NE(expression, nullptr);
    const auto& comma = std::get<AST::BinaryOp>(expression->node);
    EXPECT_EQ(comma.arg, oparg::COMMA);
    EXPECT_TRUE(std::holds_alternative<AST::Assign>(comma.lhs->node));
    EXPECT_EQ(std::get<AST::BinaryOp>(comma.rhs->node).arg, oparg::ADD);
    EXPECT_EQ(expression->src.lineno, 2);
    EXPECT_EQ(expression->src.column, 10);
    for (std::size_t i = 1; i < statements.size(); ++i) {
        const auto& caught = std::get<AST::Catch>(statements[i].node).expr;
        ASSERT_NE(caught, nullptr);
        const auto& block = std::get<AST::Block>(caught->node);
        EXPECT_EQ(block.num_locals, i == 1 ? 1u : 0u);
        const auto& contents = std::get<AST::List>(block.stmts->node).elts;
        if (i == 1) {
            ASSERT_FALSE(contents.empty());
            EXPECT_TRUE(std::holds_alternative<AST::Assign>(contents.back().node));
        } else {
            EXPECT_TRUE(contents.empty());
        }
        EXPECT_EQ(caught->src.lineno, i + 2);
        EXPECT_EQ(caught->src.column, 9);
    }
}

TEST(LpcParserTest, ResourceDeclarationListsPreserveNodesAndOrder) {
    for (const auto* source : {
        "mount (\"console\");",
        "mount (\"console\", \"weather.lpc\" => Weather, \"storage\");"
    }) {
        SCOPED_TRACE(source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(source));
        const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
        ASSERT_EQ(defs.size(), 1u);
        const auto& mount = std::get<AST::Mount>(defs.front().node);
        ASSERT_NE(mount.res_decl, nullptr);
        const auto& declarations = std::get<AST::List>(mount.res_decl->node).elts;
        const bool multiple = std::string_view(source).find(',') != std::string_view::npos;
        ASSERT_EQ(declarations.size(), multiple ? 3u : 1u);
        EXPECT_EQ(std::get<AST::StringLiteral>(declarations[0].node).idx,
                  compiler.get_consts()->find("console"));
        if (multiple) {
            const auto& entry = std::get<AST::ResourceDecl>(declarations[1].node).entry;
            ASSERT_NE(entry, nullptr);
            EXPECT_EQ(entry->name, "Weather");
            ASSERT_TRUE(entry->value.has_value());
            EXPECT_EQ(std::get<uint32_t>(*entry->value), 1u);
            EXPECT_EQ(std::get<AST::StringLiteral>(declarations[2].node).idx,
                      compiler.get_consts()->find("storage"));
            EXPECT_LT(declarations[0].src.column, declarations[1].src.column);
            EXPECT_LT(declarations[1].src.column, declarations[2].src.column);
        }
    }
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
    for (const auto& source : {"\n  int string x;", "\n  $", "void f() { return missing; }"}) {
        SCOPED_TRACE(source);
        ASSERT_FALSE(compiler.parse(source));
        ASSERT_FALSE(compiler.messages().empty());
        const auto& message = compiler.messages().back();
        const auto prefix = "example.lpc:" + std::to_string(message.lineno)
            + ":" + std::to_string(message.column) + ": ";
        EXPECT_EQ(message.text.find(prefix), 0);
    }

    compiler.set_filename("other.lpc");
    ASSERT_FALSE(compiler.parse("\n  $"));
    EXPECT_EQ(compiler.messages().back().text, "other.lpc:2:3: Unexpected character");

    compiler.set_filename("");
    ASSERT_FALSE(compiler.parse("\n  $"));
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

TEST(LpcParserTest, InheritanceUsesResourceDeclarationsOnlyForExplicitAliases) {
    for (bool aliased : {false, true}) {
        for (bool modified : {false, true}) {
            const std::string source = std::string(modified ? "public nomask " : "") +
                "inherit " + "\"/std/\" \"parent\" " + (aliased ? "=> base::parent;" : ";");
            SCOPED_TRACE(source);
            LpcCompiler compiler;
            ASSERT_TRUE(compiler.parse(source));
            const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
            ASSERT_EQ(defs.size(), 1u);
            const auto& inheritance = std::get<AST::Inheritance>(defs[0].node);
            ASSERT_NE(inheritance.res_decl, nullptr);
            const ASTNode* path = inheritance.res_decl.get();
            EXPECT_EQ(compiler.get_globals()->lookup("/std/parent"), nullptr);
            if (aliased) {
                const auto& resource = std::get<AST::ResourceDecl>(path->node);
                const auto& entry = resource.entry;
                ASSERT_NE(entry, nullptr);
                EXPECT_EQ(entry->name, "base::parent");
                EXPECT_EQ(compiler.get_globals()->lookup(entry->name), entry);
                ASSERT_TRUE(entry->value.has_value());
                EXPECT_EQ(std::get<uint32_t>(*entry->value), 1u);
                EXPECT_EQ(entry->signature.b.data_type, T_RESOURCE);
                EXPECT_EQ(entry->signature.b.is_public, modified);
                EXPECT_EQ(entry->signature.b.is_nomask, modified);
                path = resource.path.get();
            } else {
                EXPECT_EQ(compiler.get_globals()->size(), 0u);
            }
            ASSERT_NE(path, nullptr);
            EXPECT_EQ(std::get<AST::StringLiteral>(path->node).idx,
                      compiler.get_consts()->find("/std/parent"));
            EXPECT_EQ(inheritance.signature.b.is_public, modified);
            EXPECT_EQ(inheritance.signature.b.is_nomask, modified);
            auto program = compiler.generate_code();
            ASSERT_NE(program, nullptr);
            EXPECT_EQ(program->num_mounts(), aliased ? 2u : 1u);
            EXPECT_EQ(defs[0].src.lineno, 1);
            EXPECT_EQ(defs[0].src.column, 1);
        }
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
        if (!*prefix) {
            const auto& call = std::get<AST::FunctionCall>(statements.front().node);
            EXPECT_EQ(call.entry->name, "f");
            continue; // regular function call without resource acquisition
        }
        const auto& call = std::get<AST::ResourceCall>(statements.front().node);
        ASSERT_NE(call.acquisition, nullptr);
        const auto& acquisition = std::get<AST::ResourceAcquire>(call.acquisition->node);
        EXPECT_EQ(acquisition.res_name_idx, compiler.get_consts()->find("f")); // resource name is interned in the constants table
        EXPECT_EQ(acquisition.context, *prefix == '.' ? AST::ResourceAcquire::Context::INHERITANCE :
            *prefix == '-' ? AST::ResourceAcquire::Context::OBJECT : AST::ResourceAcquire::Context::MODULE);
        const auto& arguments = std::get<AST::List>(call.arguments->node).elts;
        ASSERT_EQ(arguments.size(), 2);
        EXPECT_EQ(std::get<AST::Variable>(arguments[0].node).entry->name, "x");
        EXPECT_TRUE(std::holds_alternative<AST::KeywordArgument>(arguments[1].node));
    }
}

TEST(LpcParserTest, UsingRequiresAMountAlias) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        void f() {
            mount ("module" => Module);
            using (Module) { ::call(); }
        }
    )"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& body = std::get<AST::Block>(std::get<AST::FunctionDef>(defs[0].node).body->node);
    const auto& statements = std::get<AST::List>(body.stmts->node).elts;
    ASSERT_EQ(statements.size(), 2u);
    const auto& using_stmt = std::get<AST::Using>(statements[1].node);
    ASSERT_NE(using_stmt.mount, nullptr);
    EXPECT_EQ(using_stmt.mount->name, "Module");

    for (const char* source : {
        "int f() { int value; using (value) ; }",
        "inherit \"parent\" => Parent; int f() { using (Parent) ; }"
    }) {
        SCOPED_TRACE(source);
        LpcCompiler invalid;
        EXPECT_FALSE(invalid.parse(source));
        ASSERT_FALSE(invalid.messages().empty());
        EXPECT_NE(invalid.messages().back().text.find("using currently requires a mount alias"),
                  std::string::npos);
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
        "int f(); void g(int x) { f((x = 1), key = x + 2, other = 3); f(x = 4); }"));
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
        const auto& elements = std::holds_alternative<AST::ArrayLiteral>(result.node)
            ? std::get<AST::ArrayLiteral>(result.node).elts : std::get<AST::MappingLiteral>(result.node).elts;
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
    const auto& elements = std::get<AST::ArrayLiteral>(result.node).elts;
    ASSERT_EQ(elements.size(), 1);
    const auto& pair = std::get<AST::MappingLiteral>(elements[0].node).elts;
    ASSERT_EQ(pair.size(), 2);
    EXPECT_EQ(std::get<AST::Variable>(pair[0].node).entry->name, "a");
    const auto& addition = std::get<AST::BinaryOp>(pair[1].node);
    EXPECT_EQ(addition.arg, oparg::ADD);
    ASSERT_NE(addition.lhs, nullptr);
    ASSERT_NE(addition.rhs, nullptr);
    EXPECT_EQ(std::get<AST::Variable>(addition.lhs->node).entry->name, "b");
    EXPECT_TRUE(std::holds_alternative<AST::NumericConst>(addition.rhs->node));
}


TEST(LpcParserTest, MountSlotsOverlayScopesRelativeToEachFunction) {
    LpcCompiler compiler;
    compiler.use_fabric(mount_test_fabric({"global", "late"}));
    ASSERT_TRUE(compiler.parse(R"(
        mount ("global" => Global);
        void f() {
            mount ("outer" => Outer);
            { mount ("inner" => Inner, "deep" => Deep); }
            { mount ("reuse" => Reuse); }
            for (;;) mount ("loop" => Loop);
            mount ("after" => After);
        }
        inherit "base" => Base;
        void g() { mount ("other" => Other); }
        mount ("late" => Late);
    )"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    auto slot = [](const ASTNode& node, std::size_t index = 0) {
        const auto& mount = std::get<AST::Mount>(node.node);
        const auto& decls = std::get<AST::List>(mount.res_decl->node).elts;
        const auto& entry = std::get<AST::ResourceDecl>(decls.at(index).node).entry;
        EXPECT_EQ(entry->signature.b.data_type, T_RESOURCE);
        return std::get<uint32_t>(*entry->value);
    };
    auto statements = [](const ASTNode& node) -> const std::vector<ASTNode>& {
        return std::get<AST::List>(std::get<AST::Block>(node.node).stmts->node).elts;
    };
    EXPECT_EQ(slot(defs[0]), 1u);
    EXPECT_EQ(std::get<uint32_t>(*std::get<AST::ResourceDecl>(std::get<AST::Inheritance>(defs[2].node).res_decl->node).entry->value), 2u);
    EXPECT_EQ(slot(defs[4]), 3u);
    const auto& body = statements(*std::get<AST::FunctionDef>(defs[1].node).body);
    EXPECT_EQ(slot(body[0]), 0u);
    EXPECT_EQ(slot(statements(body[1])[0]), 1u);
    EXPECT_EQ(slot(statements(body[1])[0], 1), 2u);
    EXPECT_EQ(slot(statements(body[2])[0]), 1u);
    EXPECT_EQ(slot(*std::get<AST::PreTestedLoop>(body[3].node).body), 1u);
    EXPECT_EQ(slot(body[4]), 1u);
    EXPECT_EQ(slot(statements(*std::get<AST::FunctionDef>(defs[3].node).body)[0]), 0u);
    EXPECT_EQ(compiler.get_globals()->lookup("Outer"), nullptr);
    EXPECT_EQ(compiler.get_globals()->lookup("Loop"), nullptr);
    auto program = compiler.generate_code();
    ASSERT_NE(program, nullptr);
    EXPECT_EQ(program->num_mounts(), 4u);
    EXPECT_EQ(program->num_variables(), 0u);
    EXPECT_EQ(std::get<AST::FunctionDef>(defs[1].node).num_mounts, 3u);
    EXPECT_EQ(std::get<AST::FunctionDef>(defs[3].node).num_mounts, 1u);
    EXPECT_EQ(program->get_method(0).num_mounts, 3u);
    EXPECT_EQ(program->get_method(1).num_mounts, 1u);
}

TEST(LpcParserTest, MountCountsReserveImplicitSlotAndResetBetweenParses) {
    LpcCompiler compiler;
    compiler.use_fabric(mount_test_fabric({"a", "named", "anonymous"}));
    for (const auto* source : {
        "mount (\"a\" => A); inherit \"b\" => B; int x; void f() {}",
        "mount (\"anonymous\"); int x; void f() {}",
        "inherit \"base\"; inherit \"base\"; mount (\"named\" => Named); inherit \"other\" => Other; int x; void f() {}",
        "int x; void f() {}"
    }) {
        SCOPED_TRACE(source);
        ASSERT_TRUE(compiler.parse(source));
        auto program = compiler.generate_code();
        ASSERT_NE(program, nullptr);
        EXPECT_EQ(program->num_mounts(), std::string_view(source).find("=>") != std::string_view::npos ? 3u : 1u);
        EXPECT_EQ(program->num_variables(), 1u);
    }
}


TEST(LpcParserTest, GlobalMountPathsResolveToStaticStrings) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        #define ROOT "/std/"
        mount ("implicit", ROOT "first" => First,
               (ROOT + ("sub/" + "second")) => Second);
    )"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& mount = std::get<AST::Mount>(defs[0].node);
    const auto& declarations = std::get<AST::List>(mount.res_decl->node).elts;
    ASSERT_EQ(declarations.size(), 3u);
    const auto pool = compiler.get_consts()->freeze();
    for (std::size_t i = 1; i < declarations.size(); ++i) {
        const auto& resource = std::get<AST::ResourceDecl>(declarations[i].node);
        const auto& path = std::get<AST::StringLiteral>(resource.path->node);
        EXPECT_EQ(pool->fetch_string(path.idx)->view(), i == 1 ? "/std/first" : "/std/sub/second");
    }
}

TEST(LpcParserTest, GlobalMountRejectsDynamicAndNonStringPaths) {
    for (const char* expression : {"path", "get_path()", "42", "\"prefix\" + path",
                                   "\"prefix\" + 42", "path = \"changed\""}) {
        SCOPED_TRACE(expression);
        LpcCompiler compiler;
        const std::string declarations = "string path; string get_path();\n";
        EXPECT_FALSE(compiler.parse(declarations + "mount (\"valid\" => Valid, " + expression + " => Resource);"));
        ASSERT_FALSE(compiler.messages().empty());
        EXPECT_NE(compiler.messages().back().text.find("Global mount path must be a compile-time string"),
                  std::string::npos);
        EXPECT_EQ(compiler.messages().back().lineno, 2);
    }
}

TEST(LpcParserTest, FunctionMountPathsCanRemainDynamic) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        string get_path();
        void f(string path) {
            mount (path => First, get_path() => Second, "prefix" + path => Third);
        }
    )"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    const auto& body = std::get<AST::Block>(std::get<AST::FunctionDef>(defs[1].node).body->node);
    const auto& mount = std::get<AST::Mount>(std::get<AST::List>(body.stmts->node).elts[0].node);
    const auto& declarations = std::get<AST::List>(mount.res_decl->node).elts;
    EXPECT_TRUE(std::holds_alternative<AST::Variable>(std::get<AST::ResourceDecl>(declarations[0].node).path->node));
    EXPECT_TRUE(std::holds_alternative<AST::FunctionCall>(std::get<AST::ResourceDecl>(declarations[1].node).path->node));
    EXPECT_TRUE(std::holds_alternative<AST::BinaryOp>(std::get<AST::ResourceDecl>(declarations[2].node).path->node));
}

TEST(LpcParserTest, FunctionMountRejectsUnaliasedResources) {
    for (const char* source : {
        "void f() { mount (\"local\"); }",
        "void f() { { mount (\"local\"); } }",
        "void f() { if (1) mount (\"local\"); }",
        "void f() { mount (\"named\" => Named, \"local\"); }"
    }) {
        SCOPED_TRACE(source);
        LpcCompiler compiler;
        EXPECT_FALSE(compiler.parse(source));
        ASSERT_FALSE(compiler.messages().empty());
        EXPECT_NE(compiler.messages().back().text.find(
            "Function-local mounts require an explicit resource alias"), std::string::npos);
    }
}
