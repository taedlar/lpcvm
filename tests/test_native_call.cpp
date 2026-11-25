// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_compiler.hpp"
#include "native_module.hpp"
#include "object_class.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <gtest/gtest.h>

namespace {
class TestModule : public NativeModule {
public:
    TestModule(std::shared_ptr<ManagedRuntime> fabric, std::string name, int32_t value)
        : NativeModule(fabric), name_(std::move(name)), value_(value) {
        SymbolSignature signature{};
        signature.b.is_function = 1;
        signature.b.data_type = T_INT;
        add_method("value", signature, TestModule::value);
        add_method("replace", signature, replace);
        add_method("nothing", signature, nothing);
        add_method("bad_result", signature, bad_result);
        signature.b.num_formal_params = 2;
        add_method("add", signature, add);
        signature.b.num_formal_params = 1;
        add_method("echo", signature, echo);
        signature.b.is_vararg = 1;
        add_method("options", signature, options);
        finalize_methods();
    }
    std::string_view name() const override { return name_; }
private:
    std::string name_;
    int32_t value_;
    static int value(NativeModule* self, lpcvm::DataStack* stack, lpcvm::ReturnStack*, int) {
        stack->push(static_cast<TestModule*>(self)->value_);
        return 1;
    }
    static int nothing(NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int) { return 0; }
    static int bad_result(NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int) { return 1; }
    static int echo(NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int) { return 1; }
    static int add(NativeModule*, lpcvm::DataStack* stack, lpcvm::ReturnStack*, int n) {
        const auto integer = [](const SValue& value) {
            if (const auto* number = std::get_if<uint32_t>(&value)) return static_cast<int32_t>(*number);
            if (const auto* number = std::get_if<int32_t>(&value)) return *number;
            return int32_t{0}; // absent arguments are padded with void
        };
        const auto sum = integer((*stack)[stack->size() - n]) + integer(stack->top());
        stack->pop_n(n);
        stack->push(sum);
        return 1;
    }
    static int options(NativeModule*, lpcvm::DataStack* stack, lpcvm::ReturnStack*, int n) {
        const auto count = std::get<LpcMappingRef>(stack->top())->size();
        stack->pop_n(n);
        stack->push(static_cast<int32_t>(count));
        return 1;
    }
    static int replace(NativeModule* self, lpcvm::DataStack* stack, lpcvm::ReturnStack*, int) {
        auto* module = static_cast<TestModule*>(self);
        auto fabric = module->retain_fabric();
        fabric->emplace_object(lpcvm::create_shared<TestModule>(fabric, module->name_, 99));
        // The acquired old module must stay alive even after its registration is replaced.
        stack->push(module->value_);
        return 1;
    }
};

class NativeCallTest : public testing::Test {
protected:
    std::shared_ptr<ManagedRuntime> fabric = std::make_shared<ManagedRuntime>();
    void SetUp() override {
        fabric->emplace_object(lpcvm::create_shared<TestModule>(fabric, "test", 7));
        fabric->emplace_object(lpcvm::create_shared<TestModule>(fabric, "other", 20));
    }
    SValue run(const std::string& source) {
        // Deliberately compile in a separate fabric to verify method-name re-interning.
        LpcCompiler compiler;
        if (!compiler.parse(source)) throw std::runtime_error(compiler.messages().back().text);
        auto program = compiler.generate_code();
        if (!program) throw std::runtime_error(compiler.messages().back().text);
        lpcvm::Execution code(fabric);
        if (!program->lookup_method("main", &code)) throw std::runtime_error("Missing main");
        lpcvm::DataStack stack;
        lpcvm::ReturnStack control;
        stack.push(int32_t{1234}); // caller data must survive arguments and result cleanup
        EXPECT_EQ(code.execute(control, stack), 1);
        EXPECT_EQ(stack.size(), 2u);
        EXPECT_EQ(std::get<int32_t>(stack[0]), 1234);
        EXPECT_TRUE(control.empty());
        if (std::holds_alternative<TemporaryString>(stack.top()))
            return fabric->make_shared_string(stack.top_string());
        return stack.top();
    }
};
}

TEST_F(NativeCallTest, CallsRegisteredModuleThroughDynamicLocalMount) {
    EXPECT_EQ(std::get<int32_t>(run(R"(
        int main() {
            string prefix = "te";
            mount (prefix + "st" => Module);
            return Module::add(19, Module::value());
        }
    )")), 26);
}

TEST_F(NativeCallTest, BindingDoesNotAcquireAndEachCallFindsCurrentRegistration) {
    EXPECT_EQ(std::get<int32_t>(run(R"(
        int main() {
            mount ("missing-but-never-called" => Unused);
            mount ("test" => Module) { Module::nothing(); }
            return Module::replace() + Module::value();
        }
    )")), 106);
}

TEST_F(NativeCallTest, LexicalOverlaysAndRecursiveFramesKeepTheirOwnBindings) {
    EXPECT_EQ(std::get<int32_t>(run(R"(
        int recurse(int n) {
            mount ("test" => Outer);
            if (n) {
                mount ("other" => Inner);
                return Inner::value() + recurse(n - 1) + Outer::value();
            }
            return Outer::value();
        }
        int main() {
            int result = 0;
            { mount ("test" => First); result = First::value(); }
            { mount ("other" => Second); result += Second::value(); }
            return result + recurse(2);
        }
    )")), 88);
}

TEST_F(NativeCallTest, AdjustsArgumentsAndRepresentsVoidAsAnExpressionValue) {
    EXPECT_EQ(std::get<int32_t>(run(R"(
        int main() {
            mount ("test" => Module);
            Module::nothing();
            return Module::add(5) + Module::add(1, 2, 100) + Module::options(10, label = 42);
        }
    )")), 10);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(run(R"(
        mixed main() { mount ("test" => Module); return Module::nothing(); }
    )")));
}

TEST_F(NativeCallTest, PreservesTemporaryStringResultsAcrossCallCleanup) {
    const auto result = run(R"(
        string main() {
            string part = "t";
            mount ("test" => Module);
            return Module::echo(part + "est");
        }
    )");
    EXPECT_EQ(std::get<LpcStringRef>(result)->view(), "test");
}

TEST_F(NativeCallTest, RejectsMissingResourcesMethodsAndInvalidNativeResults) {
    for (const char* source : {
        "int main() { mount (\"missing\" => M); return M::value(); }",
        "int main() { mount (\"test\" => M); return M::missing(); }",
        "int main() { mount (\"test\" => M); return M::bad_result(); }",
        "int main() { mount (42 => M); return M::value(); }",
        "int main() { mount (\"\" => M); return M::value(); }",
        "int main() { if (0) mount (\"test\" => M); return M::value(); }"
    }) {
        SCOPED_TRACE(source);
        EXPECT_THROW(run(source), std::runtime_error);
    }
    LpcObject::create(fabric, "lpc", std::make_shared<ObjectClass>());
    EXPECT_THROW(run("int main() { mount (\"lpc\" => M); return M::value(); }"), std::runtime_error);
}

TEST_F(NativeCallTest, ValidatesNativeCallAndMountOperands) {
    for (const auto& bytes : std::vector<std::vector<uint8_t>>{
        {F_CALL_NATIVE}, {F_CALL_NATIVE, 0}, {F_CALL_NATIVE, 2},
        {F_BIND_MOUNT}, {F_BIND_MOUNT, 0}, {F_LOAD_MOUNT}, {F_LOAD_MOUNT, 0}
    }) {
        lpcvm::Execution code(bytes.data(), bytes.size());
        code.fabric_ = fabric;
        lpcvm::DataStack stack;
        lpcvm::ReturnStack control;
        EXPECT_THROW(code.execute(control, stack), std::exception);
    }
}
