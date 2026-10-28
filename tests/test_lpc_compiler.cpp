// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "ast_node.hpp"
#include "bytecode_generator.hpp"
#include "lpc_compiler.hpp"
#include "object_class.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <gtest/gtest.h>

TEST(LpcCompilerTest, BytecodeGeneration) {
    // Compile a simple LPC function and generate its AST
    LpcCompiler compiler;
    EXPECT_TRUE(compiler.parse(R"(
        int f() { return 123 + 456; }
    )"));
    ASSERT_NE(compiler.get_ast(), nullptr);

    // generate the bytecode of object methods (runtime class)
    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);
    BytecodeView entry;
    ASSERT_TRUE(prog->lookup_method("f", &entry));
    SPDLOG_INFO("Method 'f' found in runtime class: length={}", entry.length());

    // Execute the bytecode for the method 'f' and check the data stack
    ReturnStack control;
    DataStack stack;
    EXPECT_EQ(entry.execute(control, stack), 1);
    ASSERT_FALSE(stack.empty());
    SPDLOG_INFO("Top of stack: variant index {}", stack.top().index());
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 579);
}

TEST(LpcCompilerTest, MultipleFunctionBytecodeRanges) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        int first();
        int first() { return 11; }
        int second() { return 22; }
    )"));

    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);
    BytecodeView first;
    BytecodeView second;
    ASSERT_TRUE(prog->lookup_method("first", &first));
    ASSERT_TRUE(prog->lookup_method("second", &second));
    EXPECT_EQ(first.length(), 4);
    EXPECT_EQ(second.length(), 4);
    EXPECT_FALSE(prog->lookup_method("missing", nullptr));

    ReturnStack control;
    DataStack stack;
    EXPECT_EQ(second.execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 1);
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 22);

    EXPECT_EQ(first.execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 2);
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 11);
}

TEST(LpcCompilerTest, BytecodeGenerationContext) {
    // construct an AST of binary addition
    ConstTable consts;
    auto c1 = consts.find_or_add(int32_t{1});
    ASTNode root = AST::BinaryOp{
        oparg::ADD,
        std::make_unique<ASTNode>(AST::NumericConst{int32_t{1}, c1}),
        std::make_unique<ASTNode>(AST::NumericConst{int32_t{1}, c1})
    };
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);

    bgen.emit_statement(root); // do a statenebt "1 + 1;" should not leave result on the stack
    bgen.emit_expression(root); // do a 1 + 1 expression, should leave result on the stack
    bgen.emit(F_BIN_OP, oparg::ADD); // push result of arg#1 + (1 + 1)
    bgen.emit(F_RETURN, 1); // return the top of the stack

    BytecodeView view(bytecode.data(), bytecode.size(), &prog);
    ReturnStack control;
    DataStack stack;
    stack.push(SValue(int32_t{1})); // argument 1
    EXPECT_EQ(view.execute(control, stack, 1), 1);
    ASSERT_EQ(stack.size(), 1);
    ASSERT_TRUE(std::holds_alternative<int32_t>(stack.top()));
    EXPECT_EQ(std::get<int32_t>(stack.top()), 3); // result of arg#1 + (1 + 1)

    // now we have 3 on top of the stack
    EXPECT_EQ(view.execute(control, stack, 1), 1);
    ASSERT_EQ(stack.size(), 1);
    ASSERT_TRUE(std::holds_alternative<int32_t>(stack.top()));
    EXPECT_EQ(std::get<int32_t>(stack.top()), 5); // result of 3 + (1 + 1) again
}

TEST(LpcCompilerTest, BlockLocalAllocation) {
    struct TestCase {
        const char* source;
        std::size_t num_params;
        std::vector<uint8_t> expected;
    };
    for (const auto& test : {
        TestCase{"void f() { {} }", 0, {}},
        TestCase{"void f(int p) { int a; { int b; { int c, d; } } { int e; } int later; }", 1,
            {F_PUSH_N, 2, F_PUSH_N, 1, F_PUSH_N, 2, F_POP_N, 2, F_POP_N, 1,
             F_PUSH_N, 1, F_POP_N, 1, F_POP_N, 2}},
        TestCase{"void f(int p) { { int a; } }", 1,
            {F_PUSH_N, 1, F_POP_N, 1}},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
        const auto& function = std::get<AST::FunctionDef>(defs.front().node);
        std::vector<uint8_t> bytecode;
        lpcvm::FunctionBuilder builder(bytecode);
        builder.emit_body(std::get<AST::Block>(function.body->node), test.num_params);
        EXPECT_EQ(bytecode, test.expected);

        builder.emit(F_RETURN, 0); // void return
        ReturnStack control;
        DataStack stack;
        stack.push(SValue(int32_t{99}));
        stack.push_n(test.num_params);
        BytecodeView view(bytecode.data(), bytecode.size(), nullptr);
        EXPECT_EQ(view.execute(control, stack, test.num_params), 0);
        ASSERT_EQ(stack.size(), 1);
        EXPECT_EQ(std::get<int32_t>(stack.top()), 99);
    }
}

TEST(LpcCompilerTest, ReturnFromNestedBlockCleansUpLocals) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse("int f(int p) { int a; { int b; return 42; } }"));
    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);
    BytecodeView entry;
    ASSERT_TRUE(prog->lookup_method("f", &entry));
    ReturnStack control;
    DataStack stack;
    stack.push(SValue(int32_t{99}));
    stack.push(SValue(int32_t{7})); // argument for the function
    EXPECT_EQ(entry.execute(control, stack, 1), 1);
    ASSERT_EQ(stack.size(), 2); // argument popped, result pushed
    EXPECT_EQ(std::get<int32_t>(stack[0]), 99);
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 42);
}
