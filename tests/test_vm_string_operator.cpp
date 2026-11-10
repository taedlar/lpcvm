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

TEST(VMExecutorTest, StringAccumulatorConcatenatesLocalStorageCombinations) {
    ManagedRuntime runtime;
    const std::string left = std::string(128, 'a') + std::string("\0b", 2);
    const std::string right = std::string("\0c", 2) + std::string(128, 'd');
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode generator(bytecode);
    generator.emit(F_PUSH_N, 1)
        .emit(F_LOAD_LOCAL, 0).emit(F_STR_BEGIN, 0)
        .emit(F_LOAD_LOCAL, 1).emit(F_STR_APPEND, 0)
        .emit(F_LOAD_LOCAL, 0).emit(F_STR_APPEND, 0)
        .emit(F_STR_FINISH, 0).emit(F_STORE_LOCAL, 2).emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 2).emit(F_RETURN, 1);
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
            EXPECT_EQ(stack.fetch_temp_string(0), "caller");
            EXPECT_EQ(stack.fetch_temp_string(1), left + right + left);
            EXPECT_TRUE(control.empty());
            stack.pop_n(2);
            EXPECT_TRUE(stack.empty());
        }
    }
}

TEST(VMExecutorTest, StringAccumulatorSupportsEmptyStringsAndRepeatedUse) {
    const uint8_t bytecode[] = {
        F_LOAD_LOCAL, 0, F_STR_BEGIN, 0,
        F_LOAD_LOCAL, 0, F_STR_APPEND, 0, F_STR_FINISH, 0,
        F_STR_BEGIN, 0, F_STR_FINISH, 0, F_RETURN, 1
    };
    lpcvm::Execution code(bytecode, sizeof(bytecode));
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    for (const auto text : {"", "value"}) {
        stack.push_temp_string(text);
        ASSERT_EQ(code.execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 1u);
        EXPECT_EQ(stack.fetch_temp_string(0), std::string(text) + text);
        stack.pop();
        EXPECT_TRUE(control.empty());
    }
}

TEST(VMExecutorTest, StringAccumulatorRejectsInvalidStatesAndArguments) {
    for (const auto& bytecode : std::vector<std::vector<uint8_t>>{
        {F_STR_APPEND, 0},
        {F_STR_FINISH, 0},
        {F_STR_BEGIN, 0, F_STR_BEGIN, 0},
        {F_STR_BEGIN, 0, F_STR_APPEND, 0},
        {F_STR_BEGIN, 0, F_RETURN, 0},
        {F_STR_BEGIN, 0, F_CALL, 0},
        {F_STR_BEGIN, 0},
        {F_STR_BEGIN, 1},
        {F_STR_BEGIN},
        {F_STR_BEGIN, 0, F_STR_APPEND, 1},
        {F_STR_BEGIN, 0, F_STR_FINISH, 1},
        {F_STR_BEGIN, 0, F_STR_APPEND},
        {F_STR_BEGIN, 0, F_STR_FINISH},
        {F_EXTENDED_ARG, 1, F_STR_BEGIN, 0}
    }) {
        lpcvm::Execution code(bytecode.data(), bytecode.size());
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        stack.push_temp_string("caller");
        stack.push_temp_string("operand");
        EXPECT_THROW(code.execute(control, stack, 1), std::runtime_error);
        EXPECT_EQ(stack.fetch_temp_string(0), "caller");
        EXPECT_TRUE(control.empty());
    }
}

TEST(VMExecutorTest, StringAccumulatorRejectsInvalidOperandsWithoutConsumingThem) {
    for (const SValue& operand : {SValue{int32_t{42}}, SValue{LpcStringRef{}}}) {
        for (bool append : {false, true}) {
            std::vector<uint8_t> bytecode;
            lpcvm::Bytecode generator(bytecode);
            if (append)
                generator.emit(F_LOAD_LOCAL, 0).emit(F_STR_BEGIN, 0);
            generator.emit(F_LOAD_LOCAL, 1).emit(append ? F_STR_APPEND : F_STR_BEGIN, 0);
            lpcvm::Execution code(bytecode.data(), bytecode.size());
            lpcvm::ReturnStack control;
            lpcvm::DataStack stack;
            stack.push_temp_string("seed");
            stack.push(operand);
            EXPECT_THROW(code.execute(control, stack, 2), std::runtime_error);
            ASSERT_EQ(stack.size(), 3u);
            ASSERT_EQ(stack.top().index(), operand.index());
            if (const auto* number = std::get_if<int32_t>(&operand))
                EXPECT_EQ(std::get<int32_t>(stack.top()), *number);
            else
                EXPECT_FALSE(std::get<LpcStringRef>(stack.top()));
            EXPECT_EQ(stack.fetch_temp_string(0), "seed");
            const uint8_t recovery[] = {F_LOAD_LOCAL, 0, F_STR_BEGIN, 0, F_STR_FINISH, 0, F_RETURN, 1};
            code.binding(nullptr, recovery, sizeof(recovery));
            ASSERT_EQ(code.execute(control, stack, 3), 1);
            EXPECT_EQ(stack.fetch_temp_string(0), "seed");
        }
    }
}

TEST(VMExecutorTest, StringAccumulatorDoesNotConsumeCallerOperands) {
    const uint8_t bytecode[] = {F_STR_BEGIN, 0};
    lpcvm::Execution code(bytecode, sizeof(bytecode));
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("caller");
    EXPECT_THROW(code.execute(control, stack), std::runtime_error);
    ASSERT_EQ(stack.size(), 1u);
    EXPECT_EQ(stack.fetch_temp_string(0), "caller");
}

TEST(VMExecutorTest, StringAccumulatorConsumesNonTailTemporary) {
    const uint8_t bytecode[] = {F_STR_BEGIN, 0, F_STR_FINISH, 0, F_RETURN, 1};
    lpcvm::Execution code(bytecode, sizeof(bytecode));
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("first");
    stack.push_temp_string("second");
    stack.push_temp_string("third");
    stack.assign_copy(0, 2);
    ASSERT_EQ(code.execute(control, stack, 1), 1);
    ASSERT_EQ(stack.size(), 3u);
    EXPECT_EQ(stack.fetch_temp_string(0), "third");
    EXPECT_EQ(stack.fetch_temp_string(1), "second");
    EXPECT_EQ(stack.fetch_temp_string(2), "third");
    EXPECT_TRUE(control.empty());
}

TEST(VMExecutorTest, LocalBufferStoreReusesUnsharedBuffer) {
    auto runtime = std::make_shared<ManagedRuntime>();
    const uint8_t bytecode[] = {
        F_LOAD_LOCAL, 0, F_STORE_LOCAL_BUFFER, 2, F_POP_N, 1,
        F_LOAD_LOCAL, 1, F_STORE_LOCAL_BUFFER, 2, F_POP_N, 1,
        F_LOAD_LOCAL, 2, F_RETURN, 1
    };
    lpcvm::Execution code(bytecode, sizeof(bytecode));
    code.fabric_ = runtime;
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string(std::string(64, 'x'));
    stack.push(runtime->make_shared_string("short"));
    stack.push_n(1);
    ASSERT_EQ(code.execute(control, stack, 3), 1);
    ASSERT_EQ(stack.size(), 1u);
    const auto& result = std::get<LpcBufferRef>(stack.top());
    EXPECT_EQ(result->data, "short");
    EXPECT_GE(result->data.capacity(), 64u); // overwritten in place
    EXPECT_EQ(result.use_count(), 1u);
    EXPECT_EQ(runtime->buffer_pool_stats().live_buffers, 1u);
}

TEST(VMExecutorTest, LocalBufferStoreCopiesOnWrite) {
    auto runtime = std::make_shared<ManagedRuntime>();
    // local 0 = "first"; push local 0; local 0 = "second"; return [old, new]
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode generator(bytecode);
    generator.emit(F_LOAD_LOCAL, 1).emit(F_STORE_LOCAL_BUFFER, 0).emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 0)
        .emit(F_LOAD_LOCAL, 2).emit(F_STORE_LOCAL_BUFFER, 0).emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 0).emit(F_STORE_LOCAL_BUFFER, 0) // self-assignment keeps the buffer
        .emit(F_RETURN, 2);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    code.fabric_ = runtime;
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_n(1);
    stack.push_temp_string("first");
    stack.push_temp_string("second");
    ASSERT_EQ(code.execute(control, stack, 3), 2);
    ASSERT_EQ(stack.size(), 2u);
    EXPECT_EQ(stack.string_at(0), "first");
    EXPECT_EQ(stack.string_at(1), "second");
    EXPECT_FALSE(std::get<LpcBufferRef>(stack[0]) == std::get<LpcBufferRef>(stack[1]));
}

TEST(VMExecutorTest, StringAccumulatorFinishesIntoLocalBuffer) {
    auto runtime = std::make_shared<ManagedRuntime>();
    // local 1 = local 0 + local 0, keep a reference, then local 1 = local 1 + local 0
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode generator(bytecode);
    generator.emit(F_LOAD_LOCAL, 0).emit(F_STR_BEGIN).emit(F_LOAD_LOCAL, 0).emit(F_STR_APPEND)
        .emit(F_STR_FINISH_LOCAL, 1)
        .emit(F_LOAD_LOCAL, 1).emit(F_STR_BEGIN).emit(F_LOAD_LOCAL, 0).emit(F_STR_APPEND)
        .emit(F_STR_FINISH_LOCAL, 1)
        .emit(F_LOAD_LOCAL, 1).emit(F_STR_BEGIN).emit(F_STR_FINISH_LOCAL, 1).emit(F_POP_N, 1)
        .emit(F_RETURN, 2);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    code.fabric_ = runtime;
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("ab");
    stack.push_n(1);
    ASSERT_EQ(code.execute(control, stack, 2), 2);
    ASSERT_EQ(stack.size(), 2u);
    const auto& first = std::get<LpcBufferRef>(stack[0]);
    const auto& second = std::get<LpcBufferRef>(stack[1]);
    EXPECT_EQ(first->data, "abab");
    EXPECT_EQ(second->data, "ababab");
    EXPECT_FALSE(first == second);

    for (const auto& invalid : std::vector<std::vector<uint8_t>>{
        {F_STR_FINISH_LOCAL, 0},
        {F_LOAD_LOCAL, 0, F_STR_BEGIN, 0, F_STR_FINISH_LOCAL},
        {F_LOAD_LOCAL, 0, F_STR_BEGIN, 0, F_STR_FINISH_LOCAL, 9}
    }) {
        lpcvm::Execution bad(invalid.data(), invalid.size());
        bad.fabric_ = runtime;
        lpcvm::DataStack args;
        args.push_temp_string("x");
        EXPECT_THROW(bad.execute(control, args, 1), std::runtime_error);
    }
    const uint8_t no_fabric[] = {F_LOAD_LOCAL, 0, F_STR_BEGIN, 0, F_STR_FINISH_LOCAL, 0};
    lpcvm::Execution bad(no_fabric, sizeof(no_fabric));
    lpcvm::DataStack args;
    args.push_temp_string("x");
    EXPECT_THROW(bad.execute(control, args, 1), std::runtime_error);
}

TEST(VMExecutorTest, LocalBufferOperandsAndFallbacks) {
    auto runtime = std::make_shared<ManagedRuntime>();
    ObjectState state(1);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode generator(bytecode);
    generator.emit(F_LOAD_LOCAL, 0).emit(F_STORE_LOCAL_BUFFER, 2).emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 2).emit(F_STORE_GLOBAL, 0).emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 2).emit(F_LOAD_LOCAL, 2).emit(F_BINARY_OP, oparg::ADD)
        .emit(F_LOAD_LOCAL, 1).emit(F_STORE_LOCAL_BUFFER, 2).emit(F_POP_N, 1) // non-string falls back
        .emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    code.fabric_ = runtime;
    code.as_with(&state);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("ab");
    stack.push(int32_t{7});
    stack.push_n(1);
    ASSERT_EQ(code.execute(control, stack, 3), 1);
    EXPECT_EQ(stack.top_string(), "abab");
    EXPECT_EQ(std::get<LpcStringRef>(state[0])->view(), "ab");

    code.fabric_ = nullptr;
    stack.pop();
    stack.push_temp_string("ab");
    stack.push(int32_t{7});
    stack.push_n(1);
    EXPECT_THROW(code.execute(control, stack, 3), std::runtime_error);
}
