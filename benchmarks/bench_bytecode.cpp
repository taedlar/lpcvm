// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "code_generator.hpp"
#include "fabric.hpp"
#include "lpc_compiler.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <benchmark/benchmark.h>

namespace {

void run_bytecode(benchmark::State& state, lpcvm::Execution& code,
    std::size_t logical_instructions,
    std::variant<std::monostate, int32_t, std::string_view> expected = std::monostate{}) {
    lpcvm::ReturnStack control;
    lpcvm::DataStack stack;
    const bool has_result = !std::holds_alternative<std::monostate>(expected);
    const auto returned = code.execute(control, stack);
    if (returned != (has_result ? 1 : 0) || stack.size() != (has_result ? 1u : 0u) ||
        !control.empty()) {
        state.SkipWithError("Unexpected return count or unbalanced VM stacks");
        return;
    }
    if (has_result) {
        bool matches = false;
        if (const auto* number = std::get_if<int32_t>(&expected)) {
            const auto* result = std::get_if<int32_t>(&stack.top());
            matches = result && *result == *number;
        } else {
            const auto text = std::get<std::string_view>(expected);
            if (std::holds_alternative<TemporaryString>(stack.top()))
                matches = stack.fetch_temp_string(0) == text;
            else if (const auto* result = std::get_if<LpcStringRef>(&stack.top()))
                matches = *result && (*result)->view() == text;
        }
        if (!matches) {
            state.SkipWithError("Unexpected execution result");
            return;
        }
        stack.pop();
    }

    for (auto _ : state) {
        auto returned = code.execute(control, stack);
        benchmark::DoNotOptimize(returned);
        if (has_result) {
            auto result = stack.pop();
            benchmark::DoNotOptimize(result);
        }
        benchmark::ClobberMemory();
    }
    if (!stack.empty() || !control.empty()) {
        state.SkipWithError("Execution leaked VM stack entries");
        return;
    }

    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(code.length() / 2));
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(code.length()));
    state.counters["logical_ops/s"] = benchmark::Counter(
        static_cast<double>(logical_instructions), benchmark::Counter::kIsIterationInvariantRate);
}

void BM_ReturnOnly(benchmark::State& state) {
    const std::vector<uint8_t> bytecode{F_RETURN, 0};
    lpcvm::Execution code(bytecode.data(), bytecode.size());
    run_bytecode(state, code, 1);
}
BENCHMARK(BM_ReturnOnly);

void BM_Dispatch(benchmark::State& state) {
    const auto pairs = static_cast<std::size_t>(state.range(0));
    const auto prefixes = static_cast<std::size_t>(state.range(1));
    const auto offset = static_cast<std::size_t>(state.range(2));
    std::vector<uint8_t> bytecode;
    bytecode.reserve(pairs * 4 * (prefixes + 1) + 2);
    for (std::size_t pair = 0; pair < pairs; ++pair) {
        for (auto opcode : {F_PUSH_N, F_POP_N}) {
            for (std::size_t prefix = 0; prefix < prefixes; ++prefix) {
                bytecode.push_back(F_EXTENDED_ARG);
                bytecode.push_back(0);
            }
            bytecode.push_back(opcode);
            bytecode.push_back(0);
        }
    }
    bytecode.push_back(F_RETURN);
    bytecode.push_back(0);

    std::vector<uint8_t> storage(bytecode.size() + 63 + offset);
    void* aligned = storage.data();
    auto space = storage.size();
    if (!std::align(64, bytecode.size() + offset, aligned, space)) {
        state.SkipWithError("Could not align bytecode storage");
        return;
    }
    auto start = static_cast<uint8_t*>(aligned) + offset;
    std::copy(bytecode.begin(), bytecode.end(), start);
    lpcvm::Execution code(start, bytecode.size());
    run_bytecode(state, code, pairs * 2 + 1);
}
BENCHMARK(BM_Dispatch)
    ->ArgsProduct({{64, 4096, 65536}, {0, 1, 2, 3}, {0, 1, 31, 63}})
    ->ArgNames({"pairs", "prefixes", "offset"});

void BM_IntegerAdd(benchmark::State& state) {
    const auto additions = static_cast<std::size_t>(state.range(0));
    ManagedRuntime fabric;
    ConstTable constants(fabric);
    const auto zero = constants.find_or_add(int32_t{0});
    const auto one = constants.find_or_add(int32_t{1});
    ObjectClass program(0, &constants);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode generator(bytecode);
    generator.emit(F_CONST_I4, zero);
    for (std::size_t addition = 0; addition < additions; ++addition)
        generator.emit(F_CONST_I4, one).emit(F_BINARY_OP, oparg::ADD);
    generator.emit(F_RETURN, 1);

    lpcvm::Execution code(bytecode.data(), bytecode.size(), &program);
    run_bytecode(state, code, additions * 2 + 2, static_cast<int32_t>(additions));
}
BENCHMARK(BM_IntegerAdd)->Arg(64)->Arg(4096)->Arg(65536)->ArgName("additions");

void BM_LocalAccess(benchmark::State& state) {
    const auto accesses = static_cast<std::size_t>(state.range(0));
    ManagedRuntime fabric;
    ConstTable constants(fabric);
    const auto value = constants.find_or_add(int32_t{42});
    ObjectClass program(0, &constants);
    std::vector<uint8_t> bytecode;
    lpcvm::Bytecode generator(bytecode);
    generator.emit(F_CONST_I4, value);
    for (std::size_t access = 0; access < accesses; ++access)
        generator.emit(F_LOAD_LOCAL, 0).emit(F_STORE_LOCAL, 0).emit(F_POP_N, 1);
    generator.emit(F_RETURN, 1);

    lpcvm::Execution code(bytecode.data(), bytecode.size(), &program);
    run_bytecode(state, code, accesses * 3 + 2, int32_t{42});
}
BENCHMARK(BM_LocalAccess)->Arg(64)->Arg(4096)->Arg(65536)->ArgName("accesses");

void string_local_concat(benchmark::State& state, bool accumulator) {
    const auto concatenations = static_cast<std::size_t>(state.range(0));
    const auto length = static_cast<std::size_t>(state.range(1));
    const auto terms = static_cast<std::size_t>(state.range(2));
    const std::string left(length, 'a');
    const std::string right(length, 'b');
    std::string expected = left;
    for (std::size_t term = 1; term < terms; ++term)
        expected += right;
    std::string source = "string run() { string left = \"" + left +
        "\"; string right = \"" + right + "\"; string result;\n";
    std::string assignment = "result = left";
    for (std::size_t term = 1; term < terms; ++term)
        assignment += " + right";
    assignment += ";\n";
    for (std::size_t operation = 0; operation < concatenations; ++operation)
        source += assignment;
    source += "return result; }";

    LpcCompiler compiler;
    if (!compiler.parse(source)) {
        state.SkipWithError("Could not parse string benchmark source");
        return;
    }
    lpcvm::CodeGenerationOptions options;
    options.use_string_accumulator = accumulator;
    auto program = compiler.generate_code(options);
    lpcvm::Execution code;
    if (!program || !program->lookup_method("run", &code)) {
        state.SkipWithError("Could not generate string benchmark bytecode");
        return;
    }
    std::size_t logical_instructions = 0;
    std::size_t accumulator_begins = 0;
    for (std::size_t offset = 0; offset < code.length(); offset += 2) {
        if (code.start_[offset] != F_EXTENDED_ARG)
            ++logical_instructions;
        if (code.start_[offset] == F_STR_BEGIN)
            ++accumulator_begins;
    }
    if (accumulator_begins != (accumulator ? concatenations : 0)) {
        state.SkipWithError("String benchmark did not use the requested emission mode");
        return;
    }
    run_bytecode(state, code, logical_instructions, std::string_view(expected));
    state.counters["concats/s"] = benchmark::Counter(
        static_cast<double>(concatenations * (terms - 1)), benchmark::Counter::kIsIterationInvariantRate);
}

void BM_StringLocalConcat(benchmark::State& state) {
    string_local_concat(state, false);
}
BENCHMARK(BM_StringLocalConcat)
    ->ArgsProduct({{64, 4096}, {0, 16, 128, 4096}, {2, 8, 32}})
    ->ArgNames({"assignments", "operand_bytes", "terms"});

void BM_StringLocalConcatAccumulator(benchmark::State& state) {
    string_local_concat(state, true);
}
BENCHMARK(BM_StringLocalConcatAccumulator)
    ->ArgsProduct({{64, 4096}, {0, 16, 128, 4096}, {2, 8, 32}})
    ->ArgNames({"assignments", "operand_bytes", "terms"});

}
