// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "lpcshell/console_module.hpp"
#include "lpc_compiler.hpp"
#include "vm_executor.hpp"
#include <gtest/gtest.h>
#include <sstream>

namespace {
class ConsoleModuleTest : public testing::Test {
protected:
    std::shared_ptr<ManagedRuntime> fabric = std::make_shared<ManagedRuntime>();
    std::istringstream input;
    std::ostringstream output;
    lpcvm::SharedPtr<ConsoleModule> module =
        lpcvm::create_shared<ConsoleModule>(fabric, input, output);
    lpcvm::DataStack stack;
    lpcvm::ReturnStack control;

    int call(std::string_view name, int argc = 0) {
        auto id = fabric->make_shared_string(name);
        return module->call_method(&*id, &stack, &control, argc);
    }
};
}

TEST_F(ConsoleModuleTest, ReadsEmptyAndUnterminatedLinesAndEof) {
    input.str("hello\n\ntail");
    for (const auto expected : {"hello", "", "tail"}) {
        ASSERT_EQ(call("read_line"), 1);
        EXPECT_EQ(stack.top_string(), expected);
        stack.pop();
    }
    ASSERT_EQ(call("read_line"), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 0);
}

TEST_F(ConsoleModuleTest, ReadsBytesIncludingWhitespaceNullAndHighBit) {
    input.str(std::string(" \n\0\xff", 4));
    for (const int32_t expected : {32, 10, 0, 255, -1, -1}) {
        ASSERT_EQ(call("read_char"), 1);
        EXPECT_EQ(std::get<int32_t>(stack.top()), expected);
        stack.pop();
    }
}

TEST_F(ConsoleModuleTest, WritesStringRepresentationsAndPreservesCallerStack) {
    stack.push(int32_t{42});
    const std::string text("a\0b", 3);
    stack.push_temp_string(text);
    ASSERT_EQ(call("write", 1), 0);
    stack.push(fabric->make_shared_string("shared"));
    ASSERT_EQ(call("write", 1), 0);
    EXPECT_EQ(output.str(), text + "shared");
    ASSERT_EQ(stack.size(), 1u);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 42);
    EXPECT_TRUE(control.empty());
}

TEST_F(ConsoleModuleTest, RejectsNonStringAndReportsStreamErrors) {
    stack.push(int32_t{42});
    EXPECT_THROW(call("write", 1), std::runtime_error);
    stack.pop();
    input.setstate(std::ios::badbit);
    EXPECT_THROW(call("read_line"), std::runtime_error);
    EXPECT_THROW(call("read_char"), std::runtime_error);
    output.setstate(std::ios::badbit);
    stack.push_temp_string("hello");
    EXPECT_THROW(call("write", 1), std::runtime_error);
}

TEST_F(ConsoleModuleTest, CallsConsoleFromLpcWithGlobalAndLocalMounts) {
    fabric->emplace_object(module);
    input.str("hello\n!");
    LpcCompiler compiler;
    compiler.use_fabric(fabric);
    ASSERT_TRUE(compiler.parse(R"(
        mount ("efun://console");
        int main() {
            mount ("efun://console" => Console);
            string line = ::read_line();
            ::write(line);
            Console::write("!");
            return Console::read_char();
        }
    )"));
    auto program = compiler.generate_code();
    ASSERT_TRUE(program);
    lpcvm::Execution code(fabric);
    ASSERT_TRUE(program->lookup_method("main", &code));
    ASSERT_EQ(code.execute(control, stack), 1);
    ASSERT_EQ(stack.size(), 1u);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 33);
    EXPECT_EQ(output.str(), "hello!");
    EXPECT_TRUE(control.empty());
}
