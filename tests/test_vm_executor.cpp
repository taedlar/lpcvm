// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cstdint>
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"

TEST(VMExecutorTest, BasicExecution) {
    std::vector<uint8_t> bytecode = {
        static_cast<uint8_t>(Opcode::F_RETURN), 0,
    };
    // Example test for basic execution
    BytecodeView bytecode_view(bytecode.data(), bytecode.size());
    EXPECT_NO_THROW(bytecode_view.execute());
}
