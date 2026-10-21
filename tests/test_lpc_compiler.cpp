// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_compiler.hpp"
#include "vm_executor.hpp"
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
    entry.execute(stack);
    ASSERT_FALSE(stack.empty());
    SPDLOG_INFO("Top of stack: variant index {}", stack.top().index());
    ASSERT_TRUE(std::holds_alternative<uint32_t>(stack.top()));
    EXPECT_EQ(std::get<uint32_t>(stack.top()), 579);
}
