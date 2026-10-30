// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cstdint>
#include "code_generator.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "source_exception.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"

class SourceExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(compiler.parse(R"(
            int f() { return 1; }
            string filenames() {
                "example.lpc";
                "included.lpc";
                "changed.lpc";
                return "later.lpc";
            }
        )"));
        object = compiler.generate_code();
        ASSERT_NE(object, nullptr);
        set_location(0, "example.lpc", 42);
        set_location(2, "included.lpc", 7);
    }

    void set_location(std::size_t offset, std::string_view filename, std::size_t line) {
        const auto index = compiler.get_consts()->find(filename);
        ASSERT_NE(index, ConstPool::INVALID_INDEX);
        object->set_source_location(offset, index, line, 1);
    }

    LpcCompiler compiler;
    std::shared_ptr<ObjectClass> object;
};

TEST_F(SourceExceptionTest, SourceLocationsReferenceInternedFileNames) {
    static_assert(std::is_same_v<decltype(ObjectClass::SourceLocation::source_file), std::string_view>);
    set_location(2, "example.lpc", 43);
    const auto first = object->lookup_source_location(object->bytecode());
    const auto second = object->lookup_source_location(object->bytecode() + 2);
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    const auto index = compiler.get_consts()->find(std::string_view("example.lpc"));
    const auto interned = object->const_pool()->fetch_string(index);

    EXPECT_EQ(first->source_file, "example.lpc");
    EXPECT_EQ(first->source_file.data(), interned.data());
    EXPECT_EQ(second->source_file.data(), interned.data());
    EXPECT_EQ(second->lineno, 43);
}

TEST_F(SourceExceptionTest, SourceLocationsRequireValidPoolEntries) {
    ObjectClass empty;
    EXPECT_THROW(empty.set_source_location(0, 0, 1, 1), std::runtime_error);
    EXPECT_THROW(object->set_source_location(0, object->const_pool()->slot_count(), 1, 1), std::out_of_range);
    const auto location = object->lookup_source_location(object->bytecode());
    ASSERT_TRUE(location.has_value());
    EXPECT_EQ(location->source_file, "example.lpc");
}

TEST_F(SourceExceptionTest, IncludesSourceLocation) {
    lpcvm::SourceException error(std::runtime_error("Stack underflow"), object.get(), object->bytecode());

    EXPECT_EQ(error.source_file(), "example.lpc");
    EXPECT_EQ(error.lineno(), 42);
    EXPECT_STREQ(error.what(), "example.lpc:42:1: Stack underflow");
}

TEST_F(SourceExceptionTest, PreservesExceptionType) {
    try {
        throw lpcvm::SourceException(std::out_of_range("Invalid index"), object.get(), object->bytecode() + 2);
    }
    catch (const std::out_of_range& error) {
        EXPECT_STREQ(error.what(), "included.lpc:7:1: Invalid index");
        return;
    }
    FAIL() << "Expected the wrapped exception to be caught as std::out_of_range";
}

TEST_F(SourceExceptionTest, OwnsSourceFileAndMessage) {
    lpcvm::SourceException error(std::runtime_error("Original message"), object.get(), object->bytecode());
    set_location(0, "changed.lpc", 99);
    object.reset();

    EXPECT_EQ(error.source_file(), "example.lpc");
    EXPECT_STREQ(error.what(), "example.lpc:42:1: Original message");
}

TEST_F(SourceExceptionTest, CopyOutlivesOriginal) {
    const auto copy = [this] {
        lpcvm::SourceException original(std::runtime_error("Failure"), object.get(), object->bytecode());
        auto copied = original;
        return copied;
    }();

    EXPECT_EQ(copy.source_file(), "example.lpc");
    EXPECT_EQ(copy.lineno(), 42);
    EXPECT_STREQ(copy.what(), "example.lpc:42:1: Failure");
}

TEST_F(SourceExceptionTest, LooksUpPrecedingEntry) {
    lpcvm::SourceException first(std::runtime_error("Failure"), object.get(), object->bytecode() + 1);
    lpcvm::SourceException second(std::runtime_error("Failure"), object.get(), object->bytecode() + 3);

    EXPECT_STREQ(first.what(), "example.lpc:42:1: Failure");
    EXPECT_STREQ(second.what(), "included.lpc:7:1: Failure");
}

TEST_F(SourceExceptionTest, MissingLocationPreservesMessage) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse("string f() { return \"later.lpc\"; }"));
    auto unmapped = compiler.generate_code();
    ASSERT_NE(unmapped, nullptr);
    lpcvm::SourceException error(std::runtime_error("Failure"), unmapped.get(), unmapped->bytecode());

    EXPECT_TRUE(error.source_file().empty());
    EXPECT_EQ(error.lineno(), 0);
    EXPECT_STREQ(error.what(), "Failure");
    const auto filename = compiler.get_consts()->find(std::string_view("later.lpc"));
    ASSERT_NE(filename, ConstPool::INVALID_INDEX);
    unmapped->set_source_location(2, filename, 10, 1);
    EXPECT_FALSE(unmapped->lookup_source_location(unmapped->bytecode() + 1).has_value());
}

TEST_F(SourceExceptionTest, InvalidContextPreservesMessage) {
    const uint8_t unrelated[] = {F_RETURN, 0};
    lpcvm::SourceException no_object(std::runtime_error("Failure"), nullptr, object->bytecode());
    lpcvm::SourceException no_pc(std::runtime_error("Failure"), object.get(), nullptr);
    lpcvm::SourceException outside(std::runtime_error("Failure"), object.get(), unrelated);
    const auto& last_function = object->get_method(1);
    lpcvm::SourceException end(std::runtime_error("Failure"), object.get(),
        object->bytecode() + last_function.bytecode_offset + last_function.bytecode_length);

    EXPECT_STREQ(no_object.what(), "Failure");
    EXPECT_STREQ(no_pc.what(), "Failure");
    EXPECT_STREQ(outside.what(), "Failure");
    EXPECT_STREQ(end.what(), "Failure");
}

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
    lpcvm::code_generator bgen(bytecode);
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
    lpcvm::code_generator bgen(bytecode);
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
    lpcvm::code_generator bgen(bytecode);
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
    lpcvm::code_generator bgen(bytecode);
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
    lpcvm::code_generator bgen(bytecode);
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
    lpcvm::code_generator bgen(bytecode);
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
        lpcvm::code_generator bgen(bytecode);
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
    lpcvm::code_generator bgen(bytecode);
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
    lpcvm::code_generator bgen(bytecode);
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
