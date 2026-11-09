// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "ast_node.hpp"
#include "code_generator.hpp"
#include "lpc_compiler.hpp"
#include "object_class.hpp"
#include "object_state.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include "fabric.hpp"
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
        lpcvm::Bytecode generator(bytecode);
        EXPECT_EQ(&generator.emit(F_RETURN, test.argument), &generator);
        EXPECT_EQ(bytecode, expected);
        EXPECT_EQ(bytecode.size() % 2, 0);
    }
}

TEST(CodeGeneratorTest, GeometricCapacityGrowth) {
    std::vector<uint8_t> bytecode;
    bytecode.reserve(3);
    lpcvm::Bytecode generator(bytecode);
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
    lpcvm::Bytecode generator(bytecode);
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
    lpcvm::Execution entry;
    ASSERT_TRUE(prog->lookup_method("f", &entry));
    SPDLOG_INFO("Method 'f' found in runtime class: length={}", entry.length());

    // Execute the bytecode for the method 'f' and check the data stack
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    EXPECT_EQ(entry.execute(control, stack), 1);
    ASSERT_FALSE(stack.empty());
    SPDLOG_INFO("Top of stack: variant index {}", stack.top().index());
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 579);
}

TEST(LpcCompilerTest, ReturnsTemporaryStringArgument) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        string identity(string value) { return value; }
        string forward(string value) { return identity(value); }
    )"));
    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);

    for (const auto* method : {"identity", "forward"}) {
        SCOPED_TRACE(method);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method(method, &entry));
        for (const auto& text : {std::string(), std::string("hello\0world", 11), std::string(128, 'x')}) {
            SCOPED_TRACE(text.size());
            lpcvm::ReturnStack control;
            lpcvm::DataStack stack;
            stack.push_temp_string("caller");
            stack.push_temp_string(text);
            ASSERT_EQ(entry.execute(control, stack, 1), 1);
            ASSERT_EQ(stack.size(), 2u);
            ASSERT_TRUE(std::holds_alternative<TemporaryString>(stack.top()));
            EXPECT_EQ(stack.fetch_temp_string(0), "caller");
            EXPECT_EQ(stack.fetch_temp_string(1), text);
            EXPECT_TRUE(control.empty());
            EXPECT_TRUE(std::holds_alternative<std::monostate>(stack.pop()));
            EXPECT_EQ(stack.fetch_temp_string(0), "caller");
        }
    }
}

TEST(LpcCompilerTest, StringConcatenationAndCompoundAssignment) {
    struct TestCase {
        const char* source;
        std::string expected;
    };
    for (const auto& test : {
        TestCase{R"(string f() { return "a" + "" + "b"; })", "ab"},
        TestCase{R"(string f() { string s = "A"; s += "B"; s += "C"; return s; })", "ABC"},
        TestCase{R"(string f() { string s = "A"; string t = s; s += "B"; return t + s; })", "AAB"},
        TestCase{R"(string f() { string s = "A"; string t = (s += "B"); s += "C"; return t + s; })", "ABABC"},
        TestCase{R"(string f() { string s = "A"; s += (s = "B"); return s; })", "AB"},
        TestCase{R"(string f() { string s = "a" + "b"; s += s; return s; })", "abab"},
        TestCase{R"(string f() { string s = ""; for (int i = 0; i < 40; i++) s += "xxx"; return s; })",
            std::string(120, 'x')},
        TestCase{R"(string f() { return "a\0b" + "\0c"; })", std::string("a\0b\0c", 5)},
        TestCase{R"(string g(string s) { return s + "!"; } string f() { return g("a" + "b") + "?"; })", "ab!?"},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method("f", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push_temp_string("caller");
        ASSERT_EQ(entry.execute(control, stack), 1);
        ASSERT_EQ(stack.size(), 2u);
        ASSERT_TRUE(std::holds_alternative<TemporaryString>(stack.top()));
        EXPECT_EQ(stack.fetch_temp_string(0), "caller");
        EXPECT_EQ(stack.fetch_temp_string(1), test.expected);
        EXPECT_TRUE(control.empty());
    }
}

TEST(LpcCompilerTest, StringCompoundAssignmentToGlobal) {
    auto runtime = std::make_shared<ManagedRuntime>();
    LpcCompiler compiler;
    compiler.use_fabric(runtime);
    ASSERT_TRUE(compiler.parse(R"(
        string message;
        string f() {
            message = "A";
            string result = (message += "B");
            message += "C";
            return result + message;
        }
    )"));
    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);
    ObjectState state(1);
    lpcvm::Execution entry(runtime);
    ASSERT_TRUE(prog->lookup_method("f", &entry));
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    ASSERT_EQ(entry.as_with(&state).execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 1u);
    EXPECT_EQ(stack.fetch_temp_string(0), "ABABC");
    ASSERT_TRUE(std::holds_alternative<LpcStringRef>(state[0]));
    EXPECT_EQ(std::get<LpcStringRef>(state[0])->view(), "ABC");
}

TEST(LpcCompilerTest, StringConstantsSurviveProgramAndRuntimeDestruction) {
    lpcvm::DataStack stack;
    {
        auto runtime = std::make_shared<ManagedRuntime>();
        LpcCompiler compiler;
        compiler.use_fabric(runtime);
        ASSERT_TRUE(compiler.parse(R"(
            string leaf() { return "hello"; }
            string caller() { string value = leaf(); return value; }
            string empty() { return ""; }
        )"));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry(runtime);
        ASSERT_TRUE(prog->lookup_method("caller", &entry));
        lpcvm::ReturnStack control;
        ASSERT_EQ(entry.execute(control, stack), 1);
        ASSERT_EQ(entry.execute(control, stack), 1);
        ASSERT_EQ(stack.size(), 2u);
        ASSERT_TRUE(std::holds_alternative<LpcStringRef>(stack[0]));
        EXPECT_EQ(std::get<LpcStringRef>(stack[0]), std::get<LpcStringRef>(stack[1]));
        EXPECT_TRUE(control.empty());
        ASSERT_TRUE(prog->lookup_method("empty", &entry));
        ASSERT_EQ(entry.execute(control, stack), 1);
        EXPECT_TRUE(std::get<LpcStringRef>(stack.top())->empty());
        EXPECT_EQ(runtime->sweep_shared_strings(), 0u);
    }
    EXPECT_EQ(std::get<LpcStringRef>(stack[0])->view(), "hello");
    EXPECT_EQ(std::get<LpcStringRef>(stack[1])->view(), "hello");
    EXPECT_TRUE(std::get<LpcStringRef>(stack[2])->empty());
}

TEST(LpcCompilerTest, AssignsStringConstantsToGlobalsAndReturnsThem) {
    auto runtime = std::make_shared<ManagedRuntime>();
    ObjectState state(1);
    {
        LpcCompiler compiler;
        compiler.use_fabric(runtime);
        ASSERT_TRUE(compiler.parse(R"(
            string message;
            string set_message() {
                "discarded";
                message = "hello";
                return message;
            }
        )"));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry(runtime);
        ASSERT_TRUE(prog->lookup_method("set_message", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        ASSERT_EQ(entry.as_with(&state).execute(control, stack), 1);
        ASSERT_EQ(stack.size(), 1u);
        ASSERT_TRUE(std::holds_alternative<LpcStringRef>(state[0]));
        EXPECT_EQ(std::get<LpcStringRef>(state[0]), std::get<LpcStringRef>(stack.top()));
    }
    EXPECT_EQ(std::get<LpcStringRef>(state[0])->view(), "hello");
    EXPECT_GT(runtime->sweep_shared_strings(), 0u);
    EXPECT_EQ(std::get<LpcStringRef>(state[0])->view(), "hello");
    state[0] = std::monostate{};
    EXPECT_GT(runtime->sweep_shared_strings(), 0u);
    EXPECT_EQ(runtime->shared_string_allocation_size(), 0u);
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
    lpcvm::Execution first;
    lpcvm::Execution second;
    ASSERT_TRUE(prog->lookup_method("first", &first));
    ASSERT_TRUE(prog->lookup_method("second", &second));
    EXPECT_EQ(first.length(), 4);
    EXPECT_EQ(second.length(), 4);
    EXPECT_FALSE(prog->lookup_method("missing", nullptr));

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
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
    lpcvm::Execution caller;
    ASSERT_TRUE(prog->lookup_method("caller", &caller));

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
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
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto c1 = consts.find_or_add(int32_t{1});
    ASTNode root = AST::BinaryOp{
        oparg::ADD,
        std::make_unique<ASTNode>(AST::NumericConst{int32_t{1}, c1}),
        std::make_unique<ASTNode>(AST::NumericConst{int32_t{1}, c1})
    };
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);

    bgen.emit_statement(root); // do a statenebt "1 + 1;" should not leave result on the stack
    bgen.emit_expression(root); // do a 1 + 1 expression, should leave result on the stack
    bgen.emit(F_BINARY_OP, oparg::ADD); // push result of arg#1 + (1 + 1)
    bgen.emit(F_RETURN, 1); // return the top of the stack

    lpcvm::Execution view(bytecode.data(), bytecode.size(), &prog);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
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

        // Generate the function body, excluding parameters from local allocation.
        std::vector<uint8_t> bytecode;
        lpcvm::Bytecode generator(bytecode);
        generator.emit_block(std::get<AST::Block>(function.body->node), test.num_params);
        EXPECT_EQ(bytecode, test.expected);

        // execute bytecode
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push(SValue(int32_t{99}));
        stack.push_n(test.num_params);
        lpcvm::Execution view(bytecode.data(), bytecode.size(), nullptr);
        EXPECT_EQ(view.execute(control, stack, test.num_params), 0);
        ASSERT_EQ(stack.size(), 1);
        EXPECT_EQ(std::get<int32_t>(stack.top()), 99);
    }
}

TEST(LpcCompilerTest, ReturnFromNestedBlockCleansUpLocals) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        int f(int p) {
            int a;
            { int b; return 42; }
        }
    )"));
    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);
    lpcvm::Execution entry;
    ASSERT_TRUE(prog->lookup_method("f", &entry));
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push(SValue(int32_t{99}));
    stack.push(SValue(int32_t{7})); // argument for the function
    EXPECT_EQ(entry.execute(control, stack, 1), 1); // call f(7) -> should return 42
    ASSERT_EQ(stack.size(), 2); // argument popped, result pushed
    EXPECT_EQ(std::get<int32_t>(stack[0]), 99);
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 42);
}

TEST(LpcCompilerTest, IfElseStatement) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        int f(int p) {
            if (p)
                return 1;
            else
                return 2;
        }
    )"));
    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);

    lpcvm::Execution entry;
    ASSERT_TRUE(prog->lookup_method("f", &entry));
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push(SValue(int32_t{99}));
    stack.push(SValue(int32_t{0})); // argument for the function
    EXPECT_EQ(entry.execute(control, stack, 1), 1); // call f(0) -> should return 2
    ASSERT_EQ(stack.size(), 2); // argument popped, result pushed
    EXPECT_EQ(std::get<int32_t>(stack[0]), 99);
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 2);
}

TEST(LpcCompilerTest, LoopStatements) {
    struct TestCase {
        const char* source;
        int32_t arg;
    };
    for (const auto& test : {
        TestCase{"int f(int p) { while (p) break; return 7; }", 1},
        TestCase{"int f(int p) { while (p) { int a; break; } return 7; }", 1},
        TestCase{"int f(int p) { while (p) { int a; { int b; break; } } return 7; }", 1},
        TestCase{"int f(int p) { while (p) return 7; return 7; }", 1},
        TestCase{"int f(int p) { while (p) return 1; return 7; }", 0},
        TestCase{"int f(int p) { do { int a; break; } while (p); return 7; }", 1},
        TestCase{"int f(int p) { do { int a; continue; } while (p); return 7; }", 0},
        TestCase{"int f(int p) { for (;p;) { int a; continue; } return 7; }", 0},
        TestCase{"int f(int p) { for (;;) { int a; break; } return 7; }", 0},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method("f", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push(SValue(int32_t{99}));
        stack.push(SValue(test.arg));
        EXPECT_EQ(entry.execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 2);
        EXPECT_EQ(std::get<int32_t>(stack[0]), 99);
        EXPECT_EQ(std::get<uint32_t>(stack.top()), 7);
    }
}

TEST(LpcCompilerTest, ShortCircuitLogicalOperators) {
    struct TestCase {
        const char* source;
        int32_t arg;
        int64_t expected;
    };
    // the division by zero must never be evaluated when the operator short-circuits
    for (const auto& test : {
        TestCase{"int f(int p) { return p && 5; }", 1, 5},
        TestCase{"int f(int p) { return p && 5; }", 0, 0},
        TestCase{"int f(int p) { return p || 5; }", 0, 5},
        TestCase{"int f(int p) { return p || 5; }", 3, 3},
        TestCase{"int f(int p) { return p && 1 / 0; }", 0, 0},
        TestCase{"int f(int p) { return p || 1 / 0; }", 2, 2},
        TestCase{"int f(int p) { return p || p && 1 / 0; }", 0, 0},
        TestCase{"int f(int p) { return (p || 0) && 7; }", 4, 7},
        TestCase{"int f(int p) { if (p && 1) return 8; return 9; }", 1, 8},
        TestCase{"int f(int p) { if (p || 0) return 8; return 9; }", 0, 9},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method("f", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push(SValue(test.arg));
        EXPECT_EQ(entry.execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 1);
        std::visit([&](const auto& v) {
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>)
                EXPECT_EQ(static_cast<int64_t>(v), test.expected);
            else
                ADD_FAILURE() << "non-numeric result";
        }, stack.top());
    }
}

TEST(LpcCompilerTest, UnaryOperators) {
    struct TestCase {
        const char* source;
        int32_t arg;
        int64_t expected;
    };
    for (const auto& test : {
        TestCase{"int f(int p) { return -p; }", 5, -5},
        TestCase{"int f(int p) { return ~p; }", 5, -6},
        TestCase{"int f(int p) { return !p; }", 0, 1},
        TestCase{"int f(int p) { return !p; }", 7, 0},
        TestCase{"int f(int p) { return - -p; }", 3, 3},
        TestCase{"int f(int p) { return !!p; }", 9, 1},
        TestCase{"int f(int p) { return -p * p; }", 4, -16},
        TestCase{"int f(int p) { return -p + 10; }", 4, 6},
        TestCase{"int f(int p) { return p - 10; }", 4, -6},
        TestCase{"int f(int p) { return 10 - p; }", 4, 6},
        TestCase{"int f(int p) { return p < 10; }", 4, 1},
        TestCase{"int f(int p) { return p == 4; }", 4, 1},
        TestCase{"int f(int p) { return (p & 6) | 1; }", 4, 5},
        TestCase{"int f(int p) { if (!p) return 1; return 2; }", 0, 1},
        TestCase{"int f(int p) { return !p && 5; }", 0, 5},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method("f", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push(SValue(test.arg));
        EXPECT_EQ(entry.execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 1);
        std::visit([&](const auto& v) {
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>)
                EXPECT_EQ(static_cast<int64_t>(v), test.expected);
            else
                ADD_FAILURE() << "non-numeric result";
        }, stack.top());
    }
}

TEST(LpcCompilerTest, TernaryOperator) {
    struct TestCase {
        const char* source;
        int32_t arg;
        int64_t expected;
    };
    // the untaken branch contains a division by zero that must not be evaluated
    for (const auto& test : {
        TestCase{"int f(int p) { return p ? 1 : 2; }", 1, 1},
        TestCase{"int f(int p) { return p ? 1 : 2; }", 0, 2},
        TestCase{"int f(int p) { return p ? 1 : 1 / 0; }", 3, 1},
        TestCase{"int f(int p) { return p ? 1 / 0 : 2; }", 0, 2},
        TestCase{"int f(int p) { return p ? 1 : p ? 2 : 3; }", 0, 3},
        TestCase{"int f(int p) { return p == 1 ? 10 : p == 2 ? 20 : 30; }", 2, 20},
        TestCase{"int f(int p) { return (p ? 1 : 2) + 10; }", 0, 12},
        TestCase{"int f(int p) { int a = p ? 5 : 6; return a; }", 1, 5},
        TestCase{"int f(int p) { int a = 0; a = p ? 5 : 6; return a; }", 0, 6},
        TestCase{"int f(int p) { return p && 1 ? 7 : 8; }", 1, 7},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method("f", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push(SValue(test.arg));
        EXPECT_EQ(entry.execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 1);
        std::visit([&](const auto& v) {
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>)
                EXPECT_EQ(static_cast<int64_t>(v), test.expected);
            else
                ADD_FAILURE() << "non-numeric result";
        }, stack.top());
    }
}

TEST(LpcCompilerTest, CompoundAssignments) {
    struct TestCase {
        const char* source;
        int32_t expected;
    };
    for (const auto& test : {
        TestCase{"int f(int p) { int a = 5; a += 3; return a; }", 8},
        TestCase{"int f(int p) { int a = 5; a -= 3; return a; }", 2},
        TestCase{"int f(int p) { int a = 5; a *= 3; return a; }", 15},
        TestCase{"int f(int p) { int a = 9; a /= 2; return a; }", 4},
        TestCase{"int f(int p) { int a = 9; a %= 4; return a; }", 1},
        TestCase{"int f(int p) { int a = 12; a &= 10; return a; }", 8},
        TestCase{"int f(int p) { int a = 12; a |= 3; return a; }", 15},
        TestCase{"int f(int p) { int a = 12; a ^= 5; return a; }", 9},
        TestCase{"int f(int p) { int a = 3; a <<= 2; return a; }", 12},
        TestCase{"int f(int p) { int a = 12; a >>= 2; return a; }", 3},
        TestCase{"int f(int p) { int a = 1; int b = (a += 4); return a + b; }", 10},
        TestCase{"int f(int p) { int a = 5; ++a; return a; }", 6},
        TestCase{"int f(int p) { int a = 5; a++; return a; }", 6},
        TestCase{"int f(int p) { int a = 5; --a; return a; }", 4},
        TestCase{"int f(int p) { int a = 5; a--; return a; }", 4},
        TestCase{"int f(int p) { int a = 5; int b = a++; return a * 10 + b; }", 65},
        TestCase{"int f(int p) { int a = 5; int b = ++a; return a * 10 + b; }", 66},
        TestCase{"int f(int p) { int a = 5; int b = a--; return a * 10 + b; }", 45},
        TestCase{"int f(int p) { int a = 5; int b = --a; return a * 10 + b; }", 44},
        TestCase{"int f(int p) { int n = 0; for (int i = 3; i; i--) n++; return n; }", 3},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method("f", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push(SValue(int32_t{0}));
        EXPECT_EQ(entry.execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 1);
        std::visit([&](const auto& v) {
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>)
                EXPECT_EQ(static_cast<int64_t>(v), test.expected);
            else
                ADD_FAILURE() << "non-numeric result";
        }, stack.top());
    }
}

TEST(LpcCompilerTest, LoopsWithAssignment) {
    struct TestCase {
        const char* source;
        uint32_t expected;
    };
    for (const auto& test : {
        TestCase{"int f(int p) { int i = 3; while (i) i = i - 1; return i; }", 0},
        TestCase{"int f(int p) { int i = 3; int n = 1; while (i) { i = i - 1; n = n + 2; } return n; }", 7},
        TestCase{"int f(int p) { int n = 1; int i = 4; for (; i; i = i - 1) n = n + 1; return n; }", 5},
        TestCase{"int f(int p) { int n = 1; int i; for (i = 4; i; i = i - 1) n = n + 1; return n; }", 5},
        TestCase{"int f(int p) { int n = 1; int i = 4; for (; i; ) { i = i - 1; n = n + 1; } return n; }", 5},
        TestCase{"int f(int p) { int n = 1; for (;;) { n = n + 1; break; } return n; }", 2},
        TestCase{"int f(int p) { int n = 1; int i = 2; for (i = 3;;i = i - 1) { if (i - 1) continue; break; } return i; }", 1},
        TestCase{"int f(int p) { int n = 1; for (int i = 3; i; i = i - 1) n = n + 1; return n; }", 4},
        TestCase{"int f(int p) { int n = 1; for (int i = 2; i; i = i - 1) { int j = 5; n = n + j; } for (int i = 2; i; i = i - 1) n = n + 1; return n; }", 13},
        TestCase{"int f(int p) { int n = 1; for (int i = 3; i; i = i - 1) { if (i - 2) continue; break; } int k = 4; return n + k; }", 5},
        TestCase{"int f(int p) { int n = 1; while (int i = 3; i) { i = i - 1; n = n + 1; } return n; }", 4},
        TestCase{"int f(int p) { int n = 1; int i = 2; while (i = 3; i) { i = i - 1; n = n + 1; } return n + i; }", 4},
        TestCase{"int f(int p) { int n = 1; while (int i = 2; i) { int j = 5; i = i - 1; n = n + j; } while (int i = 1; i) { i = i - 1; n = n + 1; } return n; }", 12},
        TestCase{"int f(int p) { int n = 1; while (int i = 3; i) { if (i - 2) { i = i - 1; continue; } break; } int k = 4; return n + k; }", 5},
        TestCase{"int f(int p) { int i = 5; do { i = i - 1; } while (i); return i; }", 0},
        TestCase{"int f(int p) { int i = 3; int n = 1; do { i = i - 1; n = n + 1; } while (i); return n; }", 4},
        TestCase{"int f(int p) { int i = 5; int n = 1; while (i) { i = i - 1; if (i - 2) continue; n = 10; break; } return n + i; }", 12},
        TestCase{"int f(int p) { int a; int b; a = b = 7; return a + b; }", 14},
    }) {
        SCOPED_TRACE(test.source);
        LpcCompiler compiler;
        ASSERT_TRUE(compiler.parse(test.source));
        auto prog = compiler.generate_code();
        ASSERT_NE(prog, nullptr);
        lpcvm::Execution entry;
        ASSERT_TRUE(prog->lookup_method("f", &entry));
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push(SValue(int32_t{99}));
        stack.push(SValue(int32_t{0}));
        EXPECT_EQ(entry.execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 2);
        EXPECT_EQ(std::get<int32_t>(stack[0]), 99);
        ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
        EXPECT_EQ(std::get<uint32_t>(stack.top()), test.expected);
    }
}

TEST(LpcCompilerTest, ForInitVariableScope) {
    LpcCompiler compiler;
    EXPECT_FALSE(compiler.parse("int f() { for (int i = 1; i; i = i - 1) {} return i; }"));
    EXPECT_FALSE(compiler.parse("int f() { while (int i = 1; i) { break; } return i; }"));
    EXPECT_TRUE(compiler.parse("int f() { int i = 1; for (int j = 1; j; j = j - 1) {} return i; }"));
}
