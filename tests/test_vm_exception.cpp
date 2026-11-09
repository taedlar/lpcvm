// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_compiler.hpp"
#include "object_class.hpp"
#include "vm_exception.hpp"
#include <gtest/gtest.h>

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
