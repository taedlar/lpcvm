// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "lpc_compiler.hpp"
#include "symbol_table.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <benchmark/benchmark.h>

namespace {

// Prepare definitions once so parsing #define directives is not timed. Each
// workload is read-only: repeated preprocess calls cannot grow the define table.
void run_expansion(benchmark::State& state, const std::string& definitions,
    const std::string& input_line, const std::string& expected_line) {
    auto predefines = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
    LpcCompiler compiler;
    compiler.use_predefines(predefines);
    compiler.preprocess(definitions);
    if (!compiler.messages().empty()) {
        state.SkipWithError("Could not prepare macro definitions");
        return;
    }

    std::string source, expected;
    for (int64_t i = 0; i < state.range(0); ++i) {
        source += input_line;
        expected += expected_line;
    }
    if (compiler.preprocess(source) != expected || !compiler.messages().empty()) {
        state.SkipWithError("Unexpected macro expansion output");
        return;
    }

    for (auto _ : state) {
        auto result = compiler.preprocess(source);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
    if (!compiler.messages().empty()) {
        state.SkipWithError("Macro expansion failed during measurement");
        return;
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(source.size()));
}

void BM_PlainText(benchmark::State& state) {
    const std::string line = "int value = unknown + 42; // unchanged\n";
    run_expansion(state, "", line, line);
}
BENCHMARK(BM_PlainText)->Arg(64)->Arg(4096)->ArgName("lines");

void BM_ObjectMacro(benchmark::State& state) {
    run_expansion(state, "#define VALUE 42\n", "int value = VALUE;\n", "int value = 42;\n");
}
BENCHMARK(BM_ObjectMacro)->Arg(64)->Arg(4096)->ArgName("lines");

void BM_FunctionMacro(benchmark::State& state) {
    run_expansion(state, "#define ADD(a,b) ((a)+(b))\n",
        "ADD(1, 2)\n", "((1)+(2))\n");
}
BENCHMARK(BM_FunctionMacro)->Arg(64)->Arg(4096)->ArgName("lines");

void BM_NestedFunctionMacro(benchmark::State& state) {
    std::string input = "VALUE";
    for (int64_t i = 0; i < state.range(1); ++i)
        input = "ID(" + input + ")";
    run_expansion(state, "#define VALUE 42\n#define ID(x) x\n", input + "\n", "42\n");
}
BENCHMARK(BM_NestedFunctionMacro)
    ->ArgsProduct({{64, 4096}, {1, 8, 32}})->ArgNames({"lines", "depth"});

void BM_ObjectMacroChain(benchmark::State& state) {
    std::string definitions = "#define M0 42\n";
    for (int64_t i = 1; i < state.range(1); ++i)
        definitions += "#define M" + std::to_string(i) + " M" + std::to_string(i - 1) + "\n";
    run_expansion(state, definitions, "M" + std::to_string(state.range(1) - 1) + "\n", "42\n");
}
BENCHMARK(BM_ObjectMacroChain)
    ->ArgsProduct({{64, 4096}, {1, 8, 32}})->ArgNames({"lines", "depth"});

void BM_RecursiveMacro(benchmark::State& state) {
    run_expansion(state, "#define A B\n#define B A\n#define LOOP(x) LOOP(x)\n",
        "A LOOP(9)\n", "A LOOP(9)\n");
}
BENCHMARK(BM_RecursiveMacro)->Arg(64)->Arg(4096)->ArgName("lines");

} // namespace
