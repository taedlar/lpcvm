// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "bench_bytecode.hpp"
#include "code_generator.hpp"
#include "lpc_compiler.hpp"

namespace {
using namespace bm;

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

} // anonymous namespace
