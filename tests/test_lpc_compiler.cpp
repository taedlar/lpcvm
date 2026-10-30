// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "ast_node.hpp"
#include "code_generator.hpp"
#include "lpc_compiler.hpp"
#include "object_class.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <gtest/gtest.h>

TEST(CodeGeneratorTest, ArgumentWidthBoundaries) {
    struct TestCase {
        uint32_t argument;
        std::vector<uint8_t> expected;
    };
    for (const auto& test : {
        TestCase{0, {F_RETURN, 0}},
        TestCase{0xff, {F_RETURN, 0xff}},
        TestCase{0x100, {F_EXTENDED_ARG, 1, F_RETURN, 0}},
        TestCase{0xffff, {F_EXTENDED_ARG, 0xff, F_RETURN, 0xff}},
        TestCase{0x10000, {F_EXTENDED_ARG, 1, F_EXTENDED_ARG, 0, F_RETURN, 0}},
        TestCase{0xffffff, {F_EXTENDED_ARG, 0xff, F_EXTENDED_ARG, 0xff, F_RETURN, 0xff}},
        TestCase{0x1000000, {F_EXTENDED_ARG, 1, F_EXTENDED_ARG, 0,
            F_EXTENDED_ARG, 0, F_RETURN, 0}},
        TestCase{0xffffffff, {F_EXTENDED_ARG, 0xff, F_EXTENDED_ARG, 0xff,
            F_EXTENDED_ARG, 0xff, F_RETURN, 0xff}},
    }) {
        SCOPED_TRACE(test.argument);
        std::vector<uint8_t> bytecode{F_RETURN, 0};
        auto expected = bytecode;
        expected.insert(expected.end(), test.expected.begin(), test.expected.end());
        lpcvm::code_generator generator(bytecode);
        EXPECT_EQ(&generator.emit(F_RETURN, test.argument), &generator);
        EXPECT_EQ(bytecode, expected);
        EXPECT_EQ(bytecode.size() % 2, 0);
    }
}

TEST(CodeGeneratorTest, GeometricCapacityGrowth) {
    std::vector<uint8_t> bytecode;
    bytecode.reserve(3);
    lpcvm::code_generator generator(bytecode);
    std::size_t allocations = 0;
    for (std::size_t instruction = 0; instruction < 4096; ++instruction) {
        const auto capacity = bytecode.capacity();
        generator.emit(F_RETURN, 0xffffffff);
        if (bytecode.capacity() != capacity) {
            ++allocations;
            EXPECT_GE(bytecode.capacity(), capacity * 2);
            EXPECT_EQ(bytecode.capacity() % 64, 0);
        }
    }
    EXPECT_LE(allocations, 10);
    ASSERT_EQ(bytecode.size(), 4096 * 8);
    const std::vector<uint8_t> expected{F_EXTENDED_ARG, 0xff, F_EXTENDED_ARG, 0xff,
        F_EXTENDED_ARG, 0xff, F_RETURN, 0xff};
    for (std::size_t offset = 0; offset < bytecode.size(); ++offset)
        ASSERT_EQ(bytecode[offset], expected[offset % expected.size()]);
}

TEST(CodeGeneratorTest, ReusesReservedCapacity) {
    std::vector<uint8_t> bytecode;
    bytecode.reserve(128);
    const auto storage = bytecode.data();
    const auto capacity = bytecode.capacity();
    lpcvm::code_generator generator(bytecode);
    for (std::size_t instruction = 0; instruction < 16; ++instruction)
        generator.emit(F_RETURN, 0xffffffff);
    EXPECT_EQ(bytecode.data(), storage);
    EXPECT_EQ(bytecode.capacity(), capacity);
    EXPECT_EQ(bytecode.size(), 128);
}

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

TEST(LpcCompilerTest, NestedFunctionCallsPassArgumentsAndReturnValues) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        int leaf(int ignored) { return 42; }
        int middle() { return leaf(7); }
        int caller() { return middle(); }
    )"));

    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);
    BytecodeView caller;
    ASSERT_TRUE(prog->lookup_method("caller", &caller));

    ReturnStack control;
    DataStack stack;
    stack.push(SValue(int32_t{99}));
    EXPECT_EQ(caller.execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 2);
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 42);
    EXPECT_EQ(std::get<int32_t>(stack[0]), 99);
    EXPECT_TRUE(control.empty());
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
    lpcvm::code_generator bgen(bytecode);

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
    LpcCompiler compiler;
    for (const auto& test : {
        TestCase{"void f() { {} }", 0, {}},
        TestCase{"void f(int p) { int a; { int b; { int c, d; } } { int e; } int later; }", 1,
            {F_PUSH_N, 2, F_PUSH_N, 1, F_PUSH_N, 2, F_POP_N, 2, F_POP_N, 1,
             F_PUSH_N, 1, F_POP_N, 1, F_POP_N, 2}},
        TestCase{"void f(int p) { { int a; } }", 1,
            {F_PUSH_N, 1, F_POP_N, 1}},
    }) {
        // parse the test source code
        SCOPED_TRACE(test.source);
        ASSERT_TRUE(compiler.parse(test.source));
        const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
        const auto& function = std::get<AST::FunctionDef>(defs.front().node);

        // code generation for the function
        std::vector<uint8_t> bytecode;
        VirtualTable vtbl{std::make_shared<ConstPool>()};
        lpcvm::code_generator codegen(bytecode);
        lpcvm::function_builder func{codegen, ConstPool::INVALID_INDEX, vtbl};
        func.build(function);
        EXPECT_EQ(bytecode, test.expected);

        // execute bytecode
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
