// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <gtest/gtest.h>
#include "code_generator.hpp"
#include "vm_executor.hpp"
#include "vm_sandbox.hpp"

TEST(VMExecutorTest, ConcatenatesAllStringStorageCombinations) {
    ManagedRuntime runtime;
    const std::string left = std::string(128, 'a') + std::string("\0b", 2);
    const std::string right = std::string("\0c", 2) + std::string(128, 'd');
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_BINARY_OP, oparg::ADD).emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size());

    for (bool left_temp : {false, true}) {
        for (bool right_temp : {false, true}) {
            SCOPED_TRACE(::testing::Message() << left_temp << " " << right_temp);
            lpcvm::ReturnStack control;
            lpcvm::DataStack stack;
            stack.push_temp_string("caller");
            if (left_temp)
                stack.push_temp_string(left);
            else
                stack.push(runtime.make_shared_string(left));
            if (right_temp)
                stack.push_temp_string(right);
            else
                stack.push(runtime.make_shared_string(right));
            ASSERT_EQ(code.execute(control, stack, 2), 1);
            ASSERT_EQ(stack.size(), 2u);
            ASSERT_TRUE(std::holds_alternative<TemporaryString>(stack.top()));
            EXPECT_EQ(stack.fetch_temp_string(0), "caller");
            EXPECT_EQ(stack.fetch_temp_string(1), left + right);
            EXPECT_TRUE(control.empty());
        }
    }
}

TEST(VMExecutorTest, ConcatenationEmptyAndErrorCases) {
    ManagedRuntime runtime;
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_BINARY_OP, oparg::ADD).emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("");
    stack.push(runtime.make_shared_string(""));
    ASSERT_EQ(code.execute(control, stack, 2), 1);
    EXPECT_TRUE(stack.fetch_temp_string(0).empty());
    stack.pop();

    stack.push_temp_string("value");
    stack.push(int32_t{1});
    EXPECT_THROW(code.execute(control, stack, 2), std::runtime_error);
    ASSERT_EQ(stack.size(), 2u);
    EXPECT_EQ(stack.fetch_temp_string(0), "value");
    EXPECT_EQ(std::get<int32_t>(stack[1]), 1);
    stack.pop_n(2);

    stack.push(runtime.make_shared_string("value"));
    stack.push(LpcStringRef{});
    EXPECT_THROW(code.execute(control, stack, 2), std::runtime_error);
    ASSERT_EQ(stack.size(), 2u);
    EXPECT_EQ(std::get<LpcStringRef>(stack[0])->view(), "value");
    stack.pop_n(2);

    EXPECT_THROW(code.execute(control, stack), std::runtime_error);
    EXPECT_TRUE(stack.empty());
    stack.push_temp_string("one");
    EXPECT_THROW(code.execute(control, stack, 1), std::runtime_error);
    ASSERT_EQ(stack.size(), 1u);
    EXPECT_EQ(stack.fetch_temp_string(0), "one");
}

TEST(VMExecutorTest, OtherStringBinaryOperationsRemainUnsupported) {
    ManagedRuntime runtime;
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_BINARY_OP, oparg::SUB).emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("a");
    stack.push(runtime.make_shared_string("b"));
    EXPECT_THROW(code.execute(control, stack, 2), std::runtime_error);
    ASSERT_EQ(stack.size(), 2u);
    EXPECT_EQ(stack.fetch_temp_string(0), "a");
    EXPECT_EQ(std::get<LpcStringRef>(stack[1])->view(), "b");
}
