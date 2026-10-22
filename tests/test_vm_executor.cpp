// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cstdint>
#include "bytecode_generator.hpp"
#include "const_pool.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"

TEST(VMExecutorTest, EmptyFunction) {
    EvaluationStack stack;
    ConstTable const_pool;
    std::vector<uint8_t> bytecode = {};
    BytecodeView code(bytecode.data(), bytecode.size(), &const_pool); // zero-length bytecode
    EXPECT_EQ(code.length(), 0);
    EXPECT_EQ(code.execute(stack), 0);
    EXPECT_EQ(stack.size(), 0);
}

TEST(VMExecutorTest, BasicExecution) {
    EvaluationStack stack;
    ConstTable const_pool;
    ConstPool::IndexType c1 = const_pool.find_or_add(int32_t{123});
    ConstPool::IndexType c2 = const_pool.find_or_add(int32_t{456});
    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_CONST_I4, c2)
        .emit(F_BIN_OP, oparg::ADD)
        .emit(F_RETURN, 1);
    // Example test for basic execution
    BytecodeView code(bytecode.data(), bytecode.size(), &const_pool);
    EXPECT_NO_THROW(code.execute(stack));
    EXPECT_EQ(stack.size(), 1);

    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123 + 456);
}

TEST(VMExecutorTest, ArgumentsPoppedUponReturn) {
    EvaluationStack stack;
    ConstTable const_pool;
    ConstPool::IndexType c1 = const_pool.find_or_add(int32_t{123});
    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_RETURN, 1);
    BytecodeView code(bytecode.data(), bytecode.size(), &const_pool);

    stack.push(SValue(int32_t{0})); // simulate an argument 0 on the stack
    EXPECT_NO_THROW(code.execute(stack, 1));
    EXPECT_EQ(stack.size(), 1); // arguments popped, returning 123 on top of the stack

    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123);
}

TEST(VMExecutorTest, ArgumentsPoppedImplicitReturn) {
    EvaluationStack stack;
    ConstTable const_pool;
    ConstPool::IndexType c1 = const_pool.find_or_add(int32_t{123});
    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_CONST_I4, c1);
    BytecodeView code(bytecode.data(), bytecode.size(), &const_pool);

    stack.push(SValue(int32_t{0})); // simulate an argument 0 on the stack
    stack.push(SValue(int32_t{1})); // simulate an argument 1 on the stack
    EXPECT_NO_THROW(code.execute(stack, 2));
    EXPECT_EQ(stack.size(), 0); // arguments popped, no return value on the stack
}