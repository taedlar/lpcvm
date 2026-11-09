// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "code_generator.hpp"
#include "object_state.hpp"
#include "vm_executor.hpp"
#include "vm_sandbox.hpp"
#include <gtest/gtest.h>

static_assert(std::is_same_v<decltype(std::declval<lpcvm::DataStack&>()[0]), const SValue&>);
static_assert(std::is_same_v<decltype(std::declval<lpcvm::DataStack&>().top()), const SValue&>);

TEST(DataStackTest, TemporaryCopiesHaveIndependentStorage) {
    lpcvm::DataStack stack;
    stack.push_temp_string(std::string_view("a\0b", 3));
    stack.push_copy(0);
    stack.pop();
    stack.push_temp_string("xyz");
    EXPECT_EQ(stack.fetch_temp_string(0), std::string_view("a\0b", 3));
    EXPECT_EQ(stack.fetch_temp_string(1), "xyz");
    EXPECT_THROW(stack.push(stack[0]), std::runtime_error);
    EXPECT_THROW(stack.assign(1, stack[0]), std::runtime_error);
}

TEST(DataStackTest, AssignmentAndRemovalFollowBufferOrder) {
    lpcvm::DataStack stack;
    stack.push_temp_string("first");
    stack.push_temp_string("second");
    stack.push_temp_string("third");
    stack.assign_copy(0, 2);
    EXPECT_EQ(stack.fetch_temp_string(0), "third");
    EXPECT_EQ(stack.fetch_temp_string(1), "second");
    EXPECT_EQ(stack.fetch_temp_string(2), "third");
    stack.assign_copy(0, 0);
    stack.pop_i(1, 1);
    EXPECT_EQ(stack.fetch_temp_string(0), "third");
    EXPECT_EQ(stack.fetch_temp_string(1), "third");
    stack.pop();
    EXPECT_EQ(stack.fetch_temp_string(0), "third");
    stack.assign(0, int32_t{42});
    EXPECT_EQ(std::get<int32_t>(stack[0]), 42);
}

TEST(DataStackTest, EmptyTemporaryStringsSurviveCompaction) {
    lpcvm::DataStack stack;
    stack.push_temp_string("");
    stack.push_temp_string("a");
    stack.push_temp_string("");
    stack.pop_i(1, 1);
    EXPECT_TRUE(stack.fetch_temp_string(0).empty());
    EXPECT_TRUE(stack.fetch_temp_string(1).empty());
    stack.push_temp_string("b");
    EXPECT_EQ(stack.fetch_temp_string(2), "b");
    stack.pop_n(3);
    EXPECT_TRUE(stack.empty());
}

TEST(DataStackTest, PushFromOwnViewAndRemoveMultipleRanges) {
    lpcvm::DataStack stack;
    const std::string text(128, 'x');
    stack.push_temp_string(text);
    stack.push_temp_string(stack.fetch_temp_string(0));
    stack.push_temp_string("last");
    stack.assign_copy(0, 2);
    stack.pop_i(1, 2);
    EXPECT_EQ(stack.fetch_temp_string(0), "last");
    stack.push_copy(0);
    EXPECT_EQ(stack.fetch_temp_string(1), "last");
}

TEST(DataStackTest, PushAndAssignmentHandleStackReferences) {
    lpcvm::DataStack stack;
    for (int32_t i = 0; i < 16; ++i)
        stack.push(i);
    stack.push(stack[0]);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 0);
    stack.assign(0, stack[1]);
    EXPECT_EQ(std::get<int32_t>(stack[0]), 1);
    EXPECT_THROW(stack.pop_n(18), std::runtime_error);
    EXPECT_EQ(stack.size(), 17u);
}

TEST(DataStackTest, TemporaryLocalCopySurvivesFrameCleanup) {
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_PUSH_N, 1)
        .emit(F_LOAD_LOCAL, 0)
        .emit(F_STORE_LOCAL, 1)
        .emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 1)
        .emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("temporary");
    ASSERT_EQ(code.execute(control, stack, 1), 1);
    ASSERT_EQ(stack.size(), 1u);
    EXPECT_EQ(stack.fetch_temp_string(0), "temporary");
}

TEST(DataStackTest, TemporaryGlobalStorePromotesAndRetainsValue) {
    ObjectState state(1);
    lpcvm::Sandbox sandbox("/mudlib");
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_STORE_GLOBAL, 0).emit(F_RETURN, 1);
    lpcvm::Execution code(&sandbox);
    code.binding(bytecode.data(), bytecode.size()).as_with(&state);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string(std::string_view("a\0b", 3));
    ASSERT_EQ(code.execute(control, stack, 1), 1);
    ASSERT_TRUE(std::holds_alternative<LpcStringRef>(state[0]));
    EXPECT_EQ(stack.fetch_temp_string(0), std::string_view("a\0b", 3));
    stack.pop();
    EXPECT_EQ(std::get<LpcStringRef>(state[0])->view(), std::string_view("a\0b", 3));
}

TEST(DataStackTest, TemporaryGlobalStoreRequiresFabric) {
    ObjectState state(1);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_STORE_GLOBAL, 0).emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("temporary");
    EXPECT_THROW(code.as_with(&state).execute(control, stack, 1), std::runtime_error);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(state[0]));
    EXPECT_EQ(stack.fetch_temp_string(0), "temporary");
}

TEST(DataStackTest, TemporaryStringIsTruthyBeforePop) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto one = consts.find_or_add(int32_t{1});
    auto two = consts.find_or_add(int32_t{2});
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    auto to_else = bgen.emit_jump(F_JUMP_IF_FALSE);
    bgen.emit(F_CONST_I4, one);
    auto to_end = bgen.emit_jump(F_JUMP);
    bgen.patch_jump(to_else);
    bgen.emit(F_CONST_I4, two);
    bgen.patch_jump(to_end);
    bgen.emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push_temp_string("");
    ASSERT_EQ(code.execute(control, stack, 1), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 1);
}
