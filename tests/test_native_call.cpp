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

TEST_F(NativeCallTest, UnaliasedGlobalMountImportsOnlyReferencedMethodsIntoSlotZero) {
    std::shared_ptr<ObjectClass> program;
    {
        LpcCompiler compiler;
        compiler.use_fabric(fabric);
        ASSERT_TRUE(compiler.parse(R"(
            mount ("other" => Other);
            int leaf() { return ::value(); }
            int main() {
                return ::add(leaf(), ::value()) + Other::value() + ::options(10, label = 42);
            }
            mount ("test");
        )"));
        program = compiler.generate_code();
        ASSERT_NE(program, nullptr);
    }
    ASSERT_EQ(program->num_mounts(), 2u);
    auto* implicit = dynamic_cast<NativeModule*>(&*program->get_mount(0));
    ASSERT_NE(implicit, nullptr);
    const auto exported = implicit->export_methods();
    ASSERT_EQ(exported.size(), 3u);
    for (const auto& entry : exported)
        EXPECT_TRUE(entry.name->view() == "value" || entry.name->view() == "add" ||
                    entry.name->view() == "options");
    const auto original = fabric->find_object("test");
    fabric->destruct_object(original);
    EXPECT_FALSE(original.expired());
    // The class owns its imports, independently of execution-fabric registrations.
    auto execution_fabric = std::make_shared<ManagedRuntime>();
    lpcvm::Execution code(execution_fabric);
    ASSERT_TRUE(program->lookup_method("main", &code));
    lpcvm::DataStack stack;
    lpcvm::ReturnStack control;
    ASSERT_EQ(code.execute(control, stack), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 36);
    EXPECT_TRUE(control.empty());
    program.reset();
    EXPECT_TRUE(original.expired());
}

TEST_F(NativeCallTest, UnaliasedGlobalMountRejectsMissingAndAmbiguousMethods) {
    for (const auto& [source, diagnostic] : std::vector<std::pair<std::string, std::string>>{
        {"int main() { return ::value(); }", "value"},
        {"mount (\"test\"); int main() { return ::missing(); }", "missing"},
        {"mount (\"test\", \"other\"); int main() { return ::value(); }", "aliased mount"},
        {"mount (\"missing\");", "not found"},
        {"mount (\"test\") {}", "do not support a body"}
    }) {
        SCOPED_TRACE(source);
        const auto before = fabric->find_object("test").use_count();
        LpcCompiler compiler;
        compiler.use_fabric(fabric);
        ASSERT_TRUE(compiler.parse(source));
        EXPECT_EQ(compiler.generate_code(), nullptr);
        ASSERT_FALSE(compiler.messages().empty());
        EXPECT_NE(compiler.messages().back().text.find(diagnostic), std::string::npos);
        EXPECT_EQ(fabric->find_object("test").use_count(), before);
    }
}

TEST_F(NativeCallTest, UnreferencedGlobalMountsDoNotImportOrRetainMethods) {
    LpcCompiler compiler;
    compiler.use_fabric(fabric);
    const auto before = fabric->find_object("test").use_count();
    ASSERT_TRUE(compiler.parse("mount (\"test\", \"other\"); int main() { return 1; }"));
    auto program = compiler.generate_code();
    ASSERT_NE(program, nullptr);
    auto* implicit = dynamic_cast<NativeModule*>(&*program->get_mount(0));
    ASSERT_NE(implicit, nullptr);
    EXPECT_TRUE(implicit->export_methods().empty());
    EXPECT_EQ(fabric->find_object("test").use_count(), before);
    // Each generation and parse starts with a fresh import set.
    ASSERT_NE(compiler.generate_code(), nullptr);
    ASSERT_TRUE(compiler.parse("int main() { return ::value(); }"));
    EXPECT_EQ(compiler.generate_code(), nullptr);
}

TEST_F(NativeCallTest, UnaliasedGlobalMountSearchesPastModulesWithoutTheMethod) {
    auto extra = lpcvm::create_shared<TestModule>(fabric, "extra", 99);
    // An earlier mount with no matching export must not prevent resolution.
    class NamedModule : public NativeModule {
    public:
        using NativeModule::NativeModule;
        std::string_view name() const override { return "limited"; }
    };
    auto limited = lpcvm::create_shared<NamedModule>(fabric);
    limited->import_method(extra, "nothing");
    limited->finalize_methods();
    fabric->emplace_object(limited);
    LpcCompiler compiler;
    compiler.use_fabric(fabric);
    ASSERT_TRUE(compiler.parse("mount (\"limited\", \"test\"); int main() { return ::value(); }"));
    auto program = compiler.generate_code();
    ASSERT_NE(program, nullptr);
    auto* implicit = dynamic_cast<NativeModule*>(&*program->get_mount(0));
    ASSERT_NE(implicit, nullptr);
    ASSERT_EQ(implicit->export_methods().size(), 1u);
    EXPECT_EQ(implicit->export_methods()[0].name->view(), "value");
}

TEST_F(NativeCallTest, ImportedMethodsRetainSourceAndRelayAcrossFabricsAndImports) {
    class DirectCallModule : public TestModule {
    public:
        using TestModule::TestModule;
        int call_method(void*, lpcvm::DataStack*, lpcvm::ReturnStack*, int) override {
            throw std::logic_error("Imported calls must use the resolved native function pointer");
        }
    };
    auto source_fabric = std::make_shared<ManagedRuntime>();
    auto source = lpcvm::create_shared<DirectCallModule>(source_fabric, "source", 42);
    lpcvm::ObjectHandle handle(source);
    auto intermediate = lpcvm::create_shared<NativeModule>(fabric);
    for (const auto& method : source->export_methods())
        intermediate->import_method(source, method.name->view());
    intermediate->finalize_methods();
    NativeModule destination(fabric);
    destination.import_method(intermediate, "value");
    destination.import_method(intermediate, "add");
    destination.import_method(intermediate, "options");
    destination.finalize_methods();
    source = {};
    intermediate = {};
    EXPECT_FALSE(handle.expired());
    lpcvm::DataStack stack;
    lpcvm::ReturnStack control;
    const auto call = [&](const char* name, int count) {
        auto identity = fabric->make_shared_string(name);
        return destination.call_method(std::addressof(*identity), &stack, &control, count);
    };
    ASSERT_EQ(call("value", 0), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 42);
    stack.pop();
    stack.push(int32_t{5});
    ASSERT_EQ(call("add", 1), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 5);
    stack.pop();
    stack.push(int32_t{10});
    stack.push(int32_t{20});
    ASSERT_EQ(call("options", 2), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 2);
    stack.pop();
    source_fabric.reset();
    EXPECT_THROW(call("value", 0), std::runtime_error);
    EXPECT_TRUE(stack.empty());
}

TEST_F(NativeCallTest, ExportsSignaturesAndRejectsInvalidImports) {
    auto source = lpcvm::create_shared<TestModule>(fabric, "source", 42);
    const auto exported = source->export_methods();
    ASSERT_EQ(exported.size(), 7u);
    const auto add = std::find_if(exported.begin(), exported.end(), [](const auto& entry) {
        return entry.name->view() == "add";
    });
    ASSERT_NE(add, exported.end());
    EXPECT_EQ(add->signature.b.num_formal_params, 2u);
    auto destination = lpcvm::create_shared<NativeModule>(fabric);
    EXPECT_THROW(destination->export_methods(), std::logic_error);
    EXPECT_THROW(destination->import_method({}, "value"), std::invalid_argument);
    EXPECT_THROW(destination->import_method(destination, "value"), std::invalid_argument);
    EXPECT_THROW(destination->import_method(source, "missing"), std::invalid_argument);
    auto unfinished = lpcvm::create_shared<NativeModule>(fabric);
    EXPECT_THROW(destination->import_method(unfinished, "value"), std::logic_error);
    destination->import_method(source, "value");
    destination->finalize_methods();
    EXPECT_THROW(destination->import_method(source, "add"), std::logic_error);
    // A back edge into an already finalized source cannot be added.
    EXPECT_THROW(source->import_method(destination, "value"), std::logic_error);
}

TEST_F(NativeCallTest, ImportedAndLocalMethodsShareOneNamespace) {
    auto source = lpcvm::create_shared<TestModule>(fabric, "source", 42);
    const auto local = +[](NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int) { return 0; };
    NativeModule destination(fabric);
    destination.add_method("local", {}, local);
    destination.import_method(source, "value");
    destination.finalize_methods();
    lpcvm::DataStack stack;
    lpcvm::ReturnStack control;
    auto identity = fabric->make_shared_string("local");
    EXPECT_EQ(destination.call_method(std::addressof(*identity), &stack, &control, 0), 0);
    for (bool imported_duplicate : {false, true}) {
        NativeModule duplicate(fabric);
        if (imported_duplicate)
            duplicate.import_method(source, "value");
        else
            duplicate.add_method("value", {}, local);
        try {
            duplicate.import_method(source, "value");
            FAIL() << "Duplicate import should fail immediately";
        } catch (const std::invalid_argument& error) {
            const std::string message = error.what();
            EXPECT_NE(message.find("value"), std::string::npos);
            EXPECT_NE(message.find("aliased mount"), std::string::npos);
        }
        // A rejected import must leave the existing table usable.
        EXPECT_NO_THROW(duplicate.finalize_methods());
        EXPECT_EQ(duplicate.export_methods().size(), 1u);
    }
    NativeModule duplicate(fabric);
    duplicate.import_method(source, "value");
    duplicate.add_method("value", {}, local);
    EXPECT_THROW(duplicate.finalize_methods(), std::invalid_argument);
}

TEST_F(NativeCallTest, GlobalMountsRetainCompileTimeObjectsAcrossCallsAndReplacement) {
    auto original = fabric->find_object("test");
    auto execution_fabric = std::make_shared<ManagedRuntime>();
    execution_fabric->emplace_object(lpcvm::create_shared<TestModule>(execution_fabric, "other", 20));
    std::shared_ptr<ObjectClass> program;
    {
        LpcCompiler compiler;
        compiler.use_fabric(fabric);
        ASSERT_TRUE(compiler.parse(R"(
            #define PREFIX "te"
            mount (PREFIX + "st" => Global, "other" => Second);
            int leaf() { return Global::value() + Second::value(); }
            int main() {
                mount ("other" => Local);
                return Global::replace() + leaf() + Global::value() + Local::value();
            }
        )"));
        program = compiler.generate_code();
        ASSERT_NE(program, nullptr) << compiler.messages().back().text;
    }
    ASSERT_EQ(program->num_mounts(), 3u);
    EXPECT_EQ(lpcvm::ObjectHandle(program->get_mount(1)), original);
    EXPECT_FALSE(program->get_mount(0));
    lpcvm::Execution code(execution_fabric);
    ASSERT_TRUE(program->lookup_method("main", &code));
    lpcvm::DataStack stack;
    lpcvm::ReturnStack control;
    ASSERT_EQ(code.execute(control, stack), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 61);
    EXPECT_FALSE(original.expired());
    fabric->destruct_object(fabric->find_object("test"));
    EXPECT_FALSE(original.expired());
    stack.pop();
    ASSERT_EQ(code.execute(control, stack), 1);
    EXPECT_EQ(std::get<int32_t>(stack.top()), 61);
    stack.pop();
    program.reset();
    EXPECT_TRUE(original.expired());
}

TEST_F(NativeCallTest, GlobalMountResolutionReportsMissingAndNonNativeModules) {
    LpcObject::create(fabric, "lpc", std::make_shared<ObjectClass>());
    for (const char* name : {"missing", "lpc", ""}) {
        SCOPED_TRACE(name);
        LpcCompiler compiler;
        compiler.use_fabric(fabric);
        ASSERT_TRUE(compiler.parse(std::string("mount (\"") + name + "\" => Module);"));
        EXPECT_EQ(compiler.generate_code(), nullptr);
        ASSERT_FALSE(compiler.messages().empty());
        EXPECT_NE(compiler.messages().back().text.find(name[0] == 'l' ? "not a native module" : "not found"),
                  std::string::npos);
    }
}

TEST_F(NativeCallTest, FailedGlobalMountCompilationReleasesEarlierBindings) {
    const auto original = fabric->find_object("test");
    const auto initial_count = original.use_count();
    LpcCompiler compiler;
    compiler.use_fabric(fabric);
    ASSERT_TRUE(compiler.parse("mount (\"test\" => First, \"missing\" => Second);"));
    EXPECT_EQ(compiler.generate_code(), nullptr);
    EXPECT_EQ(original.use_count(), initial_count);
    ASSERT_TRUE(compiler.parse("mount (\"test\" => First) { First::nothing(); }"));
    EXPECT_EQ(compiler.generate_code(), nullptr);
    EXPECT_NE(compiler.messages().back().text.find("do not support a body"), std::string::npos);
}

TEST_F(NativeCallTest, GlobalMountCallRequiresTheModulesFabricToRemainAlive) {
    std::shared_ptr<ObjectClass> program;
    lpcvm::ObjectHandle module;
    {
        auto module_fabric = std::make_shared<ManagedRuntime>();
        module = module_fabric->emplace_object(lpcvm::create_shared<TestModule>(module_fabric, "module", 3));
        LpcCompiler compiler;
        compiler.use_fabric(module_fabric);
        ASSERT_TRUE(compiler.parse("mount (\"module\" => Module); int main() { return Module::value(); }"));
        program = compiler.generate_code();
        ASSERT_NE(program, nullptr);
    }
    ASSERT_FALSE(module.expired());
    lpcvm::Execution code(fabric);
    ASSERT_TRUE(program->lookup_method("main", &code));
    lpcvm::DataStack stack;
    lpcvm::ReturnStack control;
    EXPECT_THROW(code.execute(control, stack), std::runtime_error);
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
        {F_BIND_MOUNT}, {F_BIND_MOUNT, 0}, {F_LOAD_MOUNT}, {F_LOAD_MOUNT, 0},
        {F_LOAD_GLOBAL_MOUNT}, {F_LOAD_GLOBAL_MOUNT, 0}
    }) {
        lpcvm::Execution code(bytes.data(), bytes.size());
        code.fabric_ = fabric;
        lpcvm::DataStack stack;
        lpcvm::ReturnStack control;
        EXPECT_THROW(code.execute(control, stack), std::exception);
    }
    LpcCompiler compiler;
    ASSERT_TRUE(compiler.parse("void main() {}"));
    auto program = compiler.generate_code();
    ASSERT_NE(program, nullptr);
    for (const uint8_t slot : {uint8_t{0}, uint8_t{1}}) {
        const uint8_t bytes[] = {F_LOAD_GLOBAL_MOUNT, slot};
        lpcvm::Execution code(bytes, sizeof(bytes), program.get());
        lpcvm::DataStack stack;
        lpcvm::ReturnStack control;
        EXPECT_THROW(code.execute(control, stack), std::exception);
    }
}
