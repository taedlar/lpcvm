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
    const_pool.find_or_add(int32_t{0}); // Add a dummy constant to the pool
    std::vector<uint8_t> bytecode = {
        static_cast<uint8_t>(Opcode::F_CONST_I4), 0, // Load the dummy constant
        static_cast<uint8_t>(Opcode::F_RETURN), 0,
    };
    // Example test for basic execution
    BytecodeView bytecode_view(bytecode.data(), bytecode.size(), &const_pool);
    EXPECT_NO_THROW(bytecode_view.execute(stack));
}
