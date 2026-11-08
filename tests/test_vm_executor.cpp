// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cstdint>
#include "code_generator.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "object_class.hpp"
#include "object_state.hpp"
#include "vm_exception.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include "vm_sandbox.hpp"

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

TEST(VMExecutorTest, TemporaryLocalCopySurvivesFrameCleanup) {
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

TEST(VMExecutorTest, TemporaryGlobalStorePromotesAndRetainsValue) {
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

TEST(VMExecutorTest, TemporaryGlobalStoreRequiresFabric) {
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

TEST(VMExecutorTest, TemporaryStringIsTruthyBeforePop) {
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

class SourceExceptionTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(compiler.parse(R"(
            int f() { "f"; return 1; }
            string filenames() {
                "filenames";
                "example.lpc";
                "included.lpc";
                "changed.lpc";
                return "later.lpc";
            }
        )"));
        object = compiler.generate_code();
        ASSERT_NE(object, nullptr);
        set_location(0, "example.lpc", "f", 42);
        set_location(2, "included.lpc", "filenames", 7);
    }

    void set_location(std::size_t offset, std::string_view filename, std::string_view function, std::size_t line) {
        const auto index = compiler.get_consts()->find(filename);
        const auto function_index = compiler.get_consts()->find(function);
        ASSERT_NE(index, ConstPool::INVALID_INDEX);
        ASSERT_NE(function_index, ConstPool::INVALID_INDEX);
        object->set_source_location(offset, index, function_index, line, 1);
    }

    LpcCompiler compiler;
    std::shared_ptr<ObjectClass> object;
};

TEST_F(SourceExceptionTest, SourceLocationsReferenceInternedFileNames) {
    static_assert(std::is_same_v<decltype(ObjectClass::SourceLocation::file_name), std::string_view>);
    set_location(2, "example.lpc", "f", 43);
    const auto first = object->lookup_source_location(object->bytecode());
    const auto second = object->lookup_source_location(object->bytecode() + 2);
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    const auto index = compiler.get_consts()->find(std::string_view("example.lpc"));
    const auto interned = object->const_pool()->fetch_string(index);

    EXPECT_EQ(first->file_name, "example.lpc");
    EXPECT_EQ(first->file_name, interned->view());
    EXPECT_EQ(second->file_name, interned->view());
    EXPECT_EQ(second->func_name, "f");
    EXPECT_EQ(second->lineno, 43);
}

TEST_F(SourceExceptionTest, SourceLocationsRequireValidPoolEntries) {
    ObjectClass empty;
    EXPECT_THROW(empty.set_source_location(0, 0, ConstPool::INVALID_INDEX, 1, 1), std::runtime_error);
    EXPECT_THROW(object->set_source_location(0, object->const_pool()->slot_count(), ConstPool::INVALID_INDEX, 1, 1), std::out_of_range);
    const auto location = object->lookup_source_location(object->bytecode());
    ASSERT_TRUE(location.has_value());
    EXPECT_EQ(location->file_name, "example.lpc");
}

TEST_F(SourceExceptionTest, IncludesSourceLocation) {
    lpcvm::Exception error(std::runtime_error("Stack underflow"), object.get(), object->bytecode());

    EXPECT_EQ(error.source_file(), "example.lpc");
    EXPECT_EQ(error.lineno(), 42);
    EXPECT_EQ(error.func_name(), "f");
    EXPECT_STREQ(error.what(), "example.lpc:42:1: In function `f()': Stack underflow");
}

TEST_F(SourceExceptionTest, PreservesExceptionType) {
    try {
        throw lpcvm::Exception(std::out_of_range("Invalid index"), object.get(), object->bytecode() + 2);
    }
    catch (const std::out_of_range& error) {
        EXPECT_STREQ(error.what(), "included.lpc:7:1: In function `filenames()': Invalid index");
        return;
    }
    FAIL() << "Expected the wrapped exception to be caught as std::out_of_range";
}

TEST_F(SourceExceptionTest, OwnsSourceFileAndMessage) {
    lpcvm::Exception error(std::runtime_error("Original message"), object.get(), object->bytecode());
    set_location(0, "changed.lpc", "f", 99);
    object.reset();

    EXPECT_EQ(error.source_file(), "example.lpc");
    EXPECT_STREQ(error.what(), "example.lpc:42:1: In function `f()': Original message");
}

TEST_F(SourceExceptionTest, CopyOutlivesOriginal) {
    const auto copy = [this] {
        lpcvm::Exception original(std::runtime_error("Failure"), object.get(), object->bytecode());
        auto copied = original;
        return copied;
    }();

    EXPECT_EQ(copy.source_file(), "example.lpc");
    EXPECT_EQ(copy.lineno(), 42);
    EXPECT_STREQ(copy.what(), "example.lpc:42:1: In function `f()': Failure");
}

TEST_F(SourceExceptionTest, LooksUpPrecedingEntry) {
    lpcvm::Exception first(std::runtime_error("Failure"), object.get(), object->bytecode() + 1);
    lpcvm::Exception second(std::runtime_error("Failure"), object.get(), object->bytecode() + 3);

    EXPECT_STREQ(first.what(), "example.lpc:42:1: In function `f()': Failure");
    EXPECT_STREQ(second.what(), "included.lpc:7:1: In function `filenames()': Failure");
}

TEST_F(SourceExceptionTest, MissingLocationPreservesMessage) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse("string f() { return \"later.lpc\"; }"));
    auto unmapped = compiler.generate_code();
    ASSERT_NE(unmapped, nullptr);
    const auto& function = unmapped->get_method(0);
    const auto* end = unmapped->bytecode() + function.bytecode_offset + function.bytecode_length;
    lpcvm::Exception error(std::runtime_error("Failure"), unmapped.get(), end);

    EXPECT_EQ(error.source_file(), "<unknown-file>");
    EXPECT_TRUE(error.func_name().empty());
    EXPECT_EQ(error.lineno(), 0);
    EXPECT_STREQ(error.what(), "Failure");
}

TEST_F(SourceExceptionTest, InvalidContextPreservesMessage) {
    const uint8_t unrelated[] = {F_RETURN, 0};
    lpcvm::Exception no_object(std::runtime_error("Failure"), nullptr, object->bytecode());
    lpcvm::Exception no_pc(std::runtime_error("Failure"), object.get(), nullptr);
    lpcvm::Exception outside(std::runtime_error("Failure"), object.get(), unrelated);
    const auto& last_function = object->get_method(1);
    lpcvm::Exception end(std::runtime_error("Failure"), object.get(),
        object->bytecode() + last_function.bytecode_offset + last_function.bytecode_length);

    EXPECT_STREQ(no_object.what(), "Failure");
    EXPECT_STREQ(no_pc.what(), "Failure");
    EXPECT_STREQ(outside.what(), "Failure");
    EXPECT_STREQ(end.what(), "Failure");
}

TEST(VMExecutorTest, EmptyFunction) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode = {};
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog); // zero-length bytecode
    EXPECT_EQ(code.length(), 0);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    EXPECT_EQ(code.execute(control, stack), 0);
    EXPECT_EQ(stack.size(), 0);
}

TEST(VMExecutorTest, BasicExecution) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto c1 = consts.find_or_add(int32_t{123});
    auto c2 = consts.find_or_add(int32_t{456});
    ObjectClass prog(0, &consts); 

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_CONST_I4, c2)
        .emit(F_BINARY_OP, oparg::ADD)
        .emit(F_RETURN, 1);

    // Example test for basic execution
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    EXPECT_NO_THROW(code.execute(control, stack));
    EXPECT_EQ(stack.size(), 1);

    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123 + 456);
}

TEST(VMExecutorTest, DivisionByZeroIncludesExpressionSourceLocation) {
    LpcCompiler compiler;
    compiler.set_filename("division.lpc");
    ASSERT_TRUE(compiler.parse("int divide() {\n    return 12 / 0;\n}"));
    auto object = compiler.generate_code();
    ASSERT_NE(object, nullptr);
    lpcvm::Execution code;
    ASSERT_TRUE(object->lookup_method("divide", &code));

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    try {
        code.execute(control, stack);
        FAIL() << "Expected division by zero to throw";
    }
    catch (const std::runtime_error& error) {
        EXPECT_STREQ(error.what(), "division.lpc:2:12: In function `divide()': Division by zero");
    }
}

TEST(VMExecutorTest, ReturnUnderflow) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    EXPECT_THROW(code.execute(control, stack), std::runtime_error);
}

TEST(VMExecutorTest, ArgumentsPoppedUponReturn) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto c1 = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push(SValue(int32_t{0})); // simulate an argument 0 on the stack
    EXPECT_NO_THROW(code.execute(control, stack, 1));
    EXPECT_EQ(stack.size(), 1); // arguments popped, returning 123 on top of the stack

    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123);
}

TEST(VMExecutorTest, ArgumentsPoppedImplicitReturn) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto c1 = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_CONST_I4, c1);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push(SValue(int32_t{0})); // simulate an argument 0 on the stack
    stack.push(SValue(int32_t{1})); // simulate an argument 1 on the stack
    EXPECT_NO_THROW(code.execute(control, stack, 2));
    EXPECT_EQ(stack.size(), 0); // arguments popped, no return value on the stack
}

TEST(VMExecutorTest, StringConstantPreservesEmbeddedNullAndExtendedIndex) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    for (int32_t i = 0; i < 260; ++i)
        consts.find_or_add(std::string_view(std::to_string(i)));
    const auto text = std::string_view("a\0b", 3);
    auto index = consts.find_or_add(text);
    ASSERT_GT(index & ConstPool::STRING_REF_INDEX_MASK, 255u);
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_CONST_STR, index).emit(F_RETURN, 1);
    lpcvm::Execution code(&fabric);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    ASSERT_EQ(code.binding(bytecode.data(), bytecode.size(), &prog).execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<LpcStringRef>(stack.top()));
    EXPECT_EQ(std::get<LpcStringRef>(stack.top())->view(), text);
    EXPECT_EQ(std::get<LpcStringRef>(stack.top()), lpcvm::make_shared_string(fabric, text));
}

TEST(VMExecutorTest, StringConstantDoesNotRequireExecutionFabric) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto index = consts.find_or_add(std::string_view("hello"));
    ObjectClass prog(0, &consts);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_CONST_STR, index).emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    ASSERT_EQ(code.execute(control, stack), 1);
    ASSERT_TRUE(std::holds_alternative<LpcStringRef>(stack.top()));
    EXPECT_EQ(std::get<LpcStringRef>(stack.top()), fabric.make_shared_string("hello"));
}

TEST(VMExecutorTest, StringConstantsUsePoolFabric) {
    class TrackingRuntime : public ManagedRuntime {
    public:
        int calls = 0;
        LpcStringRef make_shared_string(std::string_view value) override {
            ++calls;
            return ManagedRuntime::make_shared_string(value);
        }
    };
    auto runtime = std::make_shared<TrackingRuntime>();
    ManagedRuntime other_runtime;
    LpcCompiler compiler;
    compiler.use_fabric(runtime);
    ASSERT_TRUE(compiler.parse("string f() { return \"hello\"; }"));
    auto prog = compiler.generate_code();
    ASSERT_NE(prog, nullptr);
    const auto index = compiler.get_consts()->find(std::string_view("hello"));
    ASSERT_NE(index, ConstPool::INVALID_INDEX);
    const auto interned = prog->const_pool()->fetch_string(index);
    const auto calls_after_compile = runtime->calls;
    EXPECT_GT(calls_after_compile, 0);
    lpcvm::Execution entry(&other_runtime);
    ASSERT_TRUE(prog->lookup_method("f", &entry));
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    ASSERT_EQ(entry.execute(control, stack), 1);
    EXPECT_EQ(runtime->calls, calls_after_compile);
    EXPECT_EQ(std::get<LpcStringRef>(stack.top())->view(), "hello");
    EXPECT_EQ(std::get<LpcStringRef>(stack.top()), interned);
    EXPECT_FALSE(std::get<LpcStringRef>(stack.top()) == other_runtime.make_shared_string("hello"));
}

TEST(VMExecutorTest, StoreGlobal) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto c1 = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);
    ObjectState this_obj(1);

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_CONST_I4, c1)
        .emit(F_STORE_GLOBAL, 0)
        .emit(F_POP_N, 1)
        .emit(F_RETURN, 0);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    EXPECT_NO_THROW(code.as_with(&this_obj).execute(control, stack));
    EXPECT_EQ(stack.size(), 0);
    EXPECT_TRUE(this_obj[0].index() != std::variant_npos);
    ASSERT_TRUE(std::holds_alternative<int32_t>(this_obj[0]));
    EXPECT_EQ(std::get<int32_t>(this_obj[0]), 123);
}

TEST(VMExecutorTest, StoreGlobalStringRetainsOwnershipAfterExecution) {
    ObjectState this_obj(1);
    lpcvm::Sandbox sandbox("/mudlib");
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    const auto text = std::string_view("hello\0world", 11);
    stack.push(lpcvm::make_shared_string(sandbox, text));

    {
        std::vector<uint8_t> bytecode;
        lpcvm::Bytecode bgen(bytecode);
        bgen.emit(F_STORE_GLOBAL, 0).emit(F_RETURN, 1);
        lpcvm::Execution code(bytecode.data(), bytecode.size());
        ASSERT_EQ(code.as_with(&this_obj).execute(control, stack, 1), 1);
        ASSERT_EQ(stack.size(), 1u);
        ASSERT_TRUE(std::holds_alternative<LpcStringRef>(this_obj[0]));
        ASSERT_TRUE(std::holds_alternative<LpcStringRef>(stack.top()));
        EXPECT_EQ(std::get<LpcStringRef>(this_obj[0]), std::get<LpcStringRef>(stack.top()));
        EXPECT_EQ(std::get<LpcStringRef>(this_obj[0]).use_count(), 3u);
    }

    stack.pop_n(1);
    EXPECT_EQ(sandbox.sweep_shared_strings(), 0u);
    EXPECT_EQ(std::get<LpcStringRef>(this_obj[0])->view(), text);
    EXPECT_EQ(std::get<LpcStringRef>(this_obj[0]).use_count(), 2u);

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_LOAD_GLOBAL, 0).emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    ASSERT_EQ(code.as_with(&this_obj).execute(control, stack), 1);
    EXPECT_EQ(std::get<LpcStringRef>(stack.top()), std::get<LpcStringRef>(this_obj[0]));
    this_obj[0] = std::monostate{};
    EXPECT_EQ(sandbox.sweep_shared_strings(), 0u);
    EXPECT_EQ(std::get<LpcStringRef>(stack.top())->view(), text);
    stack.pop_n(1);
    EXPECT_GT(sandbox.sweep_shared_strings(), 0u);
    EXPECT_EQ(sandbox.shared_string_allocation_size(), 0u);
}

TEST(VMExecutorTest, StoreGlobalStringReassignmentReleasesPreviousValue) {
    ObjectState this_obj(1);
    lpcvm::Sandbox sandbox("/mudlib");
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_STORE_GLOBAL, 0).emit(F_RETURN, 0);
    lpcvm::Execution code(bytecode.data(), bytecode.size());

    stack.push(lpcvm::make_shared_string(sandbox, "first"));
    ASSERT_EQ(code.as_with(&this_obj).execute(control, stack, 1), 0);
    ASSERT_TRUE(stack.empty());
    const auto first_bytes = sandbox.shared_string_allocation_size();

    // Reassign the same shared string without losing or leaking ownership.
    stack.push(this_obj[0]);
    ASSERT_EQ(code.as_with(&this_obj).execute(control, stack, 1), 0);
    EXPECT_EQ(std::get<LpcStringRef>(this_obj[0]).use_count(), 2u);
    EXPECT_EQ(sandbox.sweep_shared_strings(), 0u);

    stack.push(lpcvm::make_shared_string(sandbox, "second"));
    ASSERT_EQ(code.execute(control, stack, 1), 0);
    EXPECT_EQ(std::get<LpcStringRef>(this_obj[0])->view(), "second");
    EXPECT_EQ(sandbox.sweep_shared_strings(), first_bytes);
    const auto second_bytes = sandbox.shared_string_allocation_size();

    stack.push(int32_t{42});
    ASSERT_EQ(code.execute(control, stack, 1), 0);
    EXPECT_EQ(std::get<int32_t>(this_obj[0]), 42);
    EXPECT_EQ(sandbox.sweep_shared_strings(), second_bytes);
    EXPECT_EQ(sandbox.shared_string_allocation_size(), 0u);
}

TEST(VMExecutorTest, LoadGlobal) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    ObjectClass prog(0, &consts);
    ObjectState this_obj(1);
    this_obj[0] = SValue(int32_t{123});

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_LOAD_GLOBAL, 0)
        .emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    EXPECT_EQ(code.as_with(&this_obj).execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 1);
    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123);
}

TEST(VMExecutorTest, GlobalAccessRequiresObjectState) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto value = consts.find_or_add(int32_t{123});
    ObjectClass prog(0, &consts);
    lpcvm::ReturnStack control;

    for (auto opcode : {F_LOAD_GLOBAL, F_STORE_GLOBAL}) {
        std::vector<uint8_t> bytecode;
        lpcvm::Bytecode bgen(bytecode);
        if (opcode == F_STORE_GLOBAL)
            bgen.emit(F_CONST_I4, value);
        bgen.emit(opcode, 0);
        lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);
        lpcvm::DataStack stack;

        EXPECT_THROW(code.execute(control, stack), std::runtime_error);
    }
}

TEST(VMExecutorTest, LoadAndStoreLocal) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto local_value = consts.find_or_add(int32_t{99});
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_PUSH_N, 1)
        .emit(F_CONST_I4, local_value)
        .emit(F_STORE_LOCAL, 1)
        .emit(F_POP_N, 1)
        .emit(F_LOAD_LOCAL, 0)
        .emit(F_LOAD_LOCAL, 1)
        .emit(F_RETURN, 2);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
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
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    ObjectClass prog(0, &consts);

    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_LOAD_LOCAL, 0)
        .emit(F_RETURN, 1);
    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);

    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    stack.push(SValue(int32_t{123}));
    for (int i = 1; i < 16; ++i)
        stack.push(SValue(int32_t{i}));

    EXPECT_EQ(code.execute(control, stack, 16), 1);
    ASSERT_EQ(stack.size(), 1);
    auto res = stack.pop();
    ASSERT_TRUE(std::holds_alternative<int32_t>(res));
    EXPECT_EQ(std::get<int32_t>(res), 123);
}

TEST(VMExecutorTest, IfElseJumps) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto c0 = consts.find_or_add(int32_t{0});
    auto c1 = consts.find_or_add(int32_t{1});
    auto c2 = consts.find_or_add(int32_t{2});
    ObjectClass prog(0, &consts);

    for (auto cond : {c0, c1}) {
        std::vector<uint8_t> bytecode;
        lpcvm::Bytecode bgen(bytecode);
        bgen.emit(F_CONST_I4, cond);
        auto to_else = bgen.emit_jump(F_JUMP_IF_FALSE);
        bgen.emit(F_CONST_I4, c1);
        auto to_end = bgen.emit_jump(F_JUMP);
        bgen.patch_jump(to_else);
        bgen.emit(F_CONST_I4, c2);
        bgen.patch_jump(to_end);
        bgen.emit(F_RETURN, 1);

        lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        EXPECT_NO_THROW(code.execute(control, stack));
        ASSERT_EQ(stack.size(), 1);
        EXPECT_EQ(std::get<int32_t>(stack.pop()), cond == c0 ? 2 : 1);
    }
}

TEST(VMExecutorTest, BackwardJump) {
    ManagedRuntime fabric;
    ConstTable consts(fabric);
    auto c1 = consts.find_or_add(int32_t{1});
    auto c3 = consts.find_or_add(int32_t{3});
    ObjectClass prog(0, &consts);

    // while (counter) counter = counter - 1;
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode bgen(bytecode);
    bgen.emit(F_PUSH_N, 1).emit(F_CONST_I4, c3).emit(F_STORE_LOCAL, 0).emit(F_POP_N, 1);
    auto loop = bgen.position();
    bgen.emit(F_LOAD_LOCAL, 0);
    auto to_end = bgen.emit_jump(F_JUMP_IF_FALSE);
    bgen.emit(F_LOAD_LOCAL, 0).emit(F_CONST_I4, c1).emit(F_BINARY_OP, oparg::SUB)
        .emit(F_STORE_LOCAL, 0).emit(F_POP_N, 1);
    bgen.emit_jump_to(F_JUMP, loop);
    bgen.patch_jump(to_end);
    bgen.emit(F_LOAD_LOCAL, 0).emit(F_RETURN, 1);

    lpcvm::Execution code(bytecode.data(), bytecode.size(), &prog);
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    EXPECT_NO_THROW(code.execute(control, stack));
    ASSERT_EQ(stack.size(), 1);
    EXPECT_EQ(std::get<int32_t>(stack.pop()), 0);
}
