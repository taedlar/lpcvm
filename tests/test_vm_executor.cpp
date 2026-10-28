// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cstdint>
#include "bytecode_generator.hpp"
#include "const_pool.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"

TEST(VMExecutorTest, EmptyFunction) {
    ConstTable consts;
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode = {};
    BytecodeView code(bytecode.data(), bytecode.size(), &prog); // zero-length bytecode
    EXPECT_EQ(code.length(), 0);
    ReturnStack control;
    DataStack stack;
    EXPECT_EQ(code.execute(control, stack), 0);
    EXPECT_EQ(stack.size(), 0);
}

TEST(VMExecutorTest, BasicExecution) {
    ConstTable consts;
    auto c1 = consts.find_or_add(int32_t{123});
    auto c2 = consts.find_or_add(int32_t{456});
    ObjectClass prog(0, &consts); 

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_CONST_I4, c2)
        .emit(F_BIN_OP, oparg::ADD)
        .emit(F_RETURN, 1);

    // Example test for basic execution
    BytecodeView code(bytecode.data(), bytecode.size(), &prog);
    ReturnStack control;
    DataStack stack;
    EXPECT_NO_THROW(code.execute(control, stack));
    EXPECT_EQ(stack.size(), 1);

    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123 + 456);
}

TEST(VMExecutorTest, ReturnUnderflow) {
    ConstTable consts;
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_RETURN, 1);
    BytecodeView code(bytecode.data(), bytecode.size(), &prog);

    ReturnStack control;
    DataStack stack;
    EXPECT_THROW(code.execute(control, stack), std::runtime_error);
}

TEST(VMExecutorTest, ArgumentsPoppedUponReturn) {
    ConstTable consts;
    auto c1 = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_RETURN, 1);
    BytecodeView code(bytecode.data(), bytecode.size(), &prog);

    ReturnStack control;
    DataStack stack;
    stack.push(SValue(int32_t{0})); // simulate an argument 0 on the stack
    EXPECT_NO_THROW(code.execute(control, stack, 1));
    EXPECT_EQ(stack.size(), 1); // arguments popped, returning 123 on top of the stack

    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123);
}

TEST(VMExecutorTest, ArgumentsPoppedImplicitReturn) {
    ConstTable consts;
    auto c1 = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_CONST_I4, c1);
    BytecodeView code(bytecode.data(), bytecode.size(), &prog);

    ReturnStack control;
    DataStack stack;
    stack.push(SValue(int32_t{0})); // simulate an argument 0 on the stack
    stack.push(SValue(int32_t{1})); // simulate an argument 1 on the stack
    EXPECT_NO_THROW(code.execute(control, stack, 2));
    EXPECT_EQ(stack.size(), 0); // arguments popped, no return value on the stack
}

TEST(VMExecutorTest, StoreGlobal) {
    ConstTable consts;
    auto c1 = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);
    ObjectState this_obj(1);

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_STORE_GLOBAL, 0)
        .emit(F_POP_N, 1)
        .emit(F_RETURN, 0);
    BytecodeView code(bytecode.data(), bytecode.size(), &prog, &this_obj);

    ReturnStack control;
    DataStack stack;
    EXPECT_NO_THROW(code.execute(control, stack));
    EXPECT_EQ(stack.size(), 0);
    EXPECT_TRUE(this_obj[0].index() != std::variant_npos);
    ASSERT_TRUE(std::holds_alternative<int32_t>(this_obj[0]));
    EXPECT_EQ(std::get<int32_t>(this_obj[0]), 123);
}

TEST(VMExecutorTest, LoadGlobal) {
    ConstTable consts;
    ObjectClass prog(0, &consts);
    ObjectState this_obj(1);
    this_obj[0] = SValue(int32_t{123});

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_LOAD_GLOBAL, 0)
        .emit(F_RETURN, 1);
    BytecodeView code(bytecode.data(), bytecode.size(), &prog, &this_obj);

    ReturnStack control;
    DataStack stack;
    EXPECT_EQ(code.execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 1);
    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123);
}

TEST(VMExecutorTest, GlobalAccessRequiresObjectState) {
    ConstTable consts;
    auto value = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);
    ReturnStack control;

    for (auto opcode : {F_LOAD_GLOBAL, F_STORE_GLOBAL}) {
        std::vector<uint8_t> bytecode;
        lpcvm::BytecodeGenerator bgen(bytecode);
        if (opcode == F_STORE_GLOBAL)
            bgen.emit(F_CONST_I4, value);
        bgen.emit(opcode, 0);
        BytecodeView code(bytecode.data(), bytecode.size(), &prog);
        DataStack stack;

        EXPECT_THROW(code.execute(control, stack), std::runtime_error);
    }
}

TEST(VMExecutorTest, LoadAndStoreLocal) {
    ConstTable consts;
    auto local_value = consts.find_or_add(int32_t{99});
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_PUSH_N, 1)
        .emit(F_CONST_I4, local_value)
        .emit(F_STORE_LOCAL, 1)
        .emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 0)
        .emit(F_LOAD_LOCAL, 1)
        .emit(F_RETURN, 2);
    BytecodeView code(bytecode.data(), bytecode.size(), &prog);

    ReturnStack control;
    DataStack stack;
    stack.push(SValue(int32_t{777})); // caller's stack value before the frame
    stack.push(SValue(int32_t{13})); // argument at local index 0
    EXPECT_EQ(code.execute(control, stack, 1), 2);
    ASSERT_EQ(stack.size(), 3);

    auto local_res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(local_res));
    EXPECT_EQ(std::get<int32_t>(local_res), 99);

    auto arg_res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(arg_res));
    EXPECT_EQ(std::get<int32_t>(arg_res), 13);

    auto caller_value = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(caller_value));
    EXPECT_EQ(std::get<int32_t>(caller_value), 777);
}

TEST(VMExecutorTest, LoadLocalWhenDataStackGrows) {
    ConstTable consts;
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::BytecodeGenerator bgen(bytecode);
    bgen.emit(F_LOAD_LOCAL, 0)
        .emit(F_RETURN, 1);
    BytecodeView code(bytecode.data(), bytecode.size(), &prog);

    ReturnStack control;
    DataStack stack;
    stack.push(SValue(int32_t{123}));
    for (int i = 1; i < 16; ++i)
        stack.push(SValue(int32_t{i}));

    EXPECT_EQ(code.execute(control, stack, 16), 1);
    ASSERT_EQ(stack.size(), 1);
    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123);
}
