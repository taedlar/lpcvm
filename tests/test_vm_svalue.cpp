// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include "lpc_compiler.hpp"
#include "object_class.hpp"
#include "vm_executor.hpp"
#include "vm_svalue.hpp"

static_assert(std::is_constructible_v<SValue, LpcStringRef>);
static_assert(sizeof(void*) != 8 || sizeof(SValue) == 16,
              "SValue must remain 16 bytes on 64-bit architectures");

TEST(SValueTest, ObjectHandleNullAndExpiredChecks) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        object context() { return self; }
        int condition(object value) { if (value) return 1; return 0; }
        int negate(object value) { return !value; }
        int double_negate(object value) { return !!value; }
        int conditional(object value) { return value ? 1 : 0; }
        int logical_and(object value) { return (value && 7) ? 1 : 0; }
        int logical_or(object value) { return (value || 0) ? 1 : 0; }
    )"));
    auto program = compiler.generate_code();
    ASSERT_NE(program, nullptr);
    auto fabric = std::make_shared<ManagedRuntime>();
    auto object = lpcvm::create_shared<LpcObject>(fabric, program);
    SValue handle;
    {
        lpcvm::Execution context;
        ASSERT_TRUE(program->lookup_method("context", &context));
        lpcvm::Execution entry(context.start_, context.length(), 0, object);
        lpcvm::ReturnStack control;
        lpcvm::DataStack stack;
        ASSERT_EQ(entry.execute(control, stack), 1);
        ASSERT_EQ(stack.size(), 1u);
        handle = stack.pop();
    }
    ASSERT_TRUE(std::holds_alternative<LpcObject::HandleType>(handle));

    for (const char* state : {"live", "expired", "null"}) {
        SCOPED_TRACE(state);
        const bool live = std::string_view(state) == "live";
        if (!live)
            object = {};
        if (std::string_view(state) == "null")
            handle = LpcObject::HandleType{};
        ASSERT_EQ(std::get<LpcObject::HandleType>(handle).expired(), !live);

        for (const char* method : {"condition", "negate", "double_negate",
                                   "conditional", "logical_and", "logical_or"}) {
            SCOPED_TRACE(method);
            lpcvm::Execution entry(fabric);
            ASSERT_TRUE(program->lookup_method(method, &entry));
            lpcvm::ReturnStack control;
            lpcvm::DataStack stack;
            stack.push(handle);
            ASSERT_EQ(entry.execute(control, stack, 1), 1);
            ASSERT_EQ(stack.size(), 1u);
            const int expected = std::string_view(method) == "negate" ? !live : live;
            std::visit([&](const auto& result) {
                if constexpr (std::is_integral_v<std::decay_t<decltype(result)>>)
                    EXPECT_EQ(result, expected);
                else
                    ADD_FAILURE() << "Expected an integer result";
            }, stack.top());
        }
    }
}

TEST(SValueTest, ObjectHandleEqualityAndInequality) {
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse(R"(
        object context() { return self; }
        int equal(object other) { return self == other; }
        int unequal(object other) { return self != other; }
        int equal_reversed(object other) { return other == self; }
        int unequal_reversed(object other) { return other != self; }
    )"));
    auto program = compiler.generate_code();
    ASSERT_NE(program, nullptr);
    auto fabric = std::make_shared<ManagedRuntime>();
    auto object = lpcvm::create_shared<LpcObject>(fabric, program);
    auto other = lpcvm::create_shared<LpcObject>(fabric, program);
    lpcvm::Execution context;
    ASSERT_TRUE(program->lookup_method("context", &context));

    // Obtain both handles by executing self in their respective object contexts.
    lpcvm::DataStack handles;
    lpcvm::ReturnStack control;
    for (const auto& owner : {object, other}) {
        lpcvm::Execution entry(context.start_, context.length(), 0, owner);
        ASSERT_EQ(entry.execute(control, handles), 1);
        ASSERT_TRUE(std::holds_alternative<LpcObject::HandleType>(handles.top()));
        auto locked = std::get<LpcObject::HandleType>(handles.top()).lock();
        ASSERT_TRUE(locked.has_value());
        EXPECT_TRUE(*locked == owner);
    }
    SValue different = handles.pop();
    SValue same = handles.pop();
    SValue empty = LpcObject::HandleType{};

    for (bool expired : {false, true}) {
        SCOPED_TRACE(expired ? "expired other object" : "live other object");
        if (expired)
            other = {};
        EXPECT_EQ(std::get<LpcObject::HandleType>(different).expired(), expired);
        for (const char* method : {"equal", "unequal", "equal_reversed", "unequal_reversed"}) {
            SCOPED_TRACE(method);
            lpcvm::Execution code;
            ASSERT_TRUE(program->lookup_method(method, &code));
            for (const auto& [operand, is_same] : {
                std::pair<const SValue*, bool>{&same, true},
                {&different, false}, {&empty, false}}) {
                SCOPED_TRACE(is_same ? "same object" : "different or empty handle");
                lpcvm::Execution entry(code.start_, code.length(), 0, object);
                lpcvm::DataStack stack;
                stack.push(*operand);
                ASSERT_EQ(entry.execute(control, stack, 1), 1);
                ASSERT_EQ(stack.size(), 1u);
                ASSERT_TRUE(std::holds_alternative<int32_t>(stack.top()));
                EXPECT_EQ(std::get<int32_t>(stack.top()),
                          method[0] == 'u' ? !is_same : is_same);
            }
        }
    }
}
