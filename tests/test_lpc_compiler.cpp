// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "ast_node.hpp"
#include "bytecode_generator.hpp"
#include "lpc_compiler.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include "vm_runtime_class.hpp"
#include <gtest/gtest.h>

TEST(LpcCompilerTest, BackendBytecodeGeneration) {
    // Compile a simple LPC function and generate its AST
    LpcCompiler compiler;
    auto root = compiler.frontend(R"(
        int f() { return 123 + 456; }
    )");
    ASSERT_NE(root.get(), nullptr);

    // Invoke the backend to generate the runtime class and bytecode
    auto rt_class = compiler.backend();
    BytecodeView entry;
    EXPECT_TRUE(rt_class->lookup_method("f", &entry));
    SPDLOG_INFO("Method 'f' found in runtime class: length={}", entry.length());

    // Execute the bytecode for the method 'f' and check the evaluation stack
    EvaluationStack stack;
    EXPECT_EQ(entry.execute(stack), 1);
    ASSERT_FALSE(stack.empty());
    SPDLOG_INFO("Top of stack: variant index {}", stack.top().index());
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 579);
}

TEST(LpcCompilerTest, BytecodeGenerationContext) {
    // construct an AST of binary addition
    ConstTable const_pool;
    auto c1 = const_pool.find_or_add(int32_t{1});
    ASTNode root = AST::BinaryOp{
        oparg::ADD,
        std::make_unique<ASTNode>(AST::NumericConst{int32_t{1}, c1}),
        std::make_unique<ASTNode>(AST::NumericConst{int32_t{1}, c1})
    };

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);

    bgen.emit_statement(root); // do a statenebt "1 + 1;" should not leave result on the stack
    bgen.emit_expression(root); // do a 1 + 1 expression, should leave result on the stack
    bgen.emit(F_BIN_OP, oparg::ADD); // push result of arg#1 + (1 + 1)
    bgen.emit(F_RETURN, 1); // return the top of the stack

    BytecodeView view(bytecode.data(), bytecode.size(), &const_pool);
    EvaluationStack stack;
    stack.push(int32_t{1}); // argument 1
    EXPECT_EQ(view.execute(stack, 1), 1);
    ASSERT_EQ(stack.size(), 1);
    ASSERT_TRUE(std::holds_alternative<int32_t>(stack.top()));
    EXPECT_EQ(std::get<int32_t>(stack.top()), 3); // result of arg#1 + (1 + 1)

    // now we have 3 on top of the stack
    EXPECT_EQ(view.execute(stack, 1), 1);
    ASSERT_EQ(stack.size(), 1);
    ASSERT_TRUE(std::holds_alternative<int32_t>(stack.top()));
    EXPECT_EQ(std::get<int32_t>(stack.top()), 5); // result of 3 + (1 + 1) again
}
