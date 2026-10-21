// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cstdint>
#include "const_pool.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"

TEST(VMExecutorTest, BasicExecution) {
    EvaluationStack stack;
    ConstTable const_pool;
    ConstPool::IndexType c1 = const_pool.find_or_add(int32_t{123});
    ConstPool::IndexType c2 = const_pool.find_or_add(int32_t{456});
    std::vector<uint8_t> bytecode = { // return 123 + 456;
        static_cast<uint8_t>(Opcode::F_CONST_I4), static_cast<uint8_t>(c1),
        static_cast<uint8_t>(Opcode::F_CONST_I4), static_cast<uint8_t>(c2),
        static_cast<uint8_t>(Opcode::F_BIN_OP), static_cast<uint8_t>(BinaryOparg::ADD),
        static_cast<uint8_t>(Opcode::F_RETURN), 1,
    };
    // Example test for basic execution
    BytecodeView bytecode_view(bytecode.data(), bytecode.size(), &const_pool);
    EXPECT_NO_THROW(bytecode_view.execute(stack));
    EXPECT_EQ(stack.size(), 1);

    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123 + 456);
}
