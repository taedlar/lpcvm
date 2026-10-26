// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "ast_node.hpp"
#include "bytecode_generator.hpp"
#include "lpc_compiler.hpp"
#include "runtime_class.hpp"
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
    auto rt_class = compiler.generate_code();
    ASSERT_NE(rt_class, nullptr);
    BytecodeView entry;
    ASSERT_TRUE(rt_class->lookup_method("f", &entry));
    SPDLOG_INFO("Method 'f' found in runtime class: length={}", entry.length());

    // Execute the bytecode for the method 'f' and check the operand stack
    OperandStack stack;
    EXPECT_EQ(entry.execute(stack), 1);
    ASSERT_FALSE(stack.empty());
    SPDLOG_INFO("Top of stack: variant index {}", stack.top().index());
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 579);
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
    SymbolTable symbols;
    RuntimeClass rt_class(consts, symbols);

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);

    bgen.emit_statement(root); // do a statenebt "1 + 1;" should not leave result on the stack
    bgen.emit_expression(root); // do a 1 + 1 expression, should leave result on the stack
    bgen.emit(F_BIN_OP, oparg::ADD); // push result of arg#1 + (1 + 1)
    bgen.emit(F_RETURN, 1); // return the top of the stack

    auto const_pool = consts.make_intern();
    BytecodeView view(bytecode.data(), bytecode.size(), &rt_class);
    OperandStack stack;
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
