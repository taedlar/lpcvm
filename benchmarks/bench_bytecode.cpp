// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "code_generator.hpp"
#include "vm_executor.hpp"
#include "vm_opcodes.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
#include <benchmark/benchmark.h>

namespace {

void run_bytecode(benchmark::State& state, BytecodeView& code,
    std::size_t logical_instructions, std::optional<int32_t> expected = std::nullopt) {
    ReturnStack control;
    DataStack stack;
    const auto returned = code.execute(control, stack);
    if (returned != (expected ? 1 : 0) || stack.size() != (expected ? 1u : 0u) ||
        !control.empty()) {
        state.SkipWithError("Unexpected return count or unbalanced VM stacks");
        return;
    }
    if (expected) {
        const auto result = stack.pop();
        if (!std::holds_alternative<int32_t>(result) || std::get<int32_t>(result) != *expected) {
            state.SkipWithError("Unexpected execution result");
            return;
        }
    }

    for (auto _ : state) {
        auto returned = code.execute(control, stack);
        benchmark::DoNotOptimize(returned);
        if (expected) {
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
    BytecodeView code(bytecode.data(), bytecode.size());
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
    BytecodeView code(start, bytecode.size());
    run_bytecode(state, code, pairs * 2 + 1);
}
BENCHMARK(BM_Dispatch)
    ->ArgsProduct({{64, 4096, 65536}, {0, 1, 2, 3}, {0, 1, 31, 63}})
    ->ArgNames({"pairs", "prefixes", "offset"});

void BM_IntegerAdd(benchmark::State& state) {
    const auto additions = static_cast<std::size_t>(state.range(0));
    ConstTable constants;
    const auto zero = constants.find_or_add(int32_t{0});
    const auto one = constants.find_or_add(int32_t{1});
    ObjectClass program(0, &constants);
    std::vector<uint8_t> bytecode;
    lpcvm::CodeGenerator generator(bytecode);
    generator.emit(F_CONST_I4, zero);
    for (std::size_t addition = 0; addition < additions; ++addition)
        generator.emit(F_CONST_I4, one).emit(F_BIN_OP, oparg::ADD);
    generator.emit(F_RETURN, 1);

    BytecodeView code(bytecode.data(), bytecode.size(), &program);
    run_bytecode(state, code, additions * 2 + 2, static_cast<int32_t>(additions));
}
BENCHMARK(BM_IntegerAdd)->Arg(64)->Arg(4096)->Arg(65536)->ArgName("additions");

void BM_LocalAccess(benchmark::State& state) {
    const auto accesses = static_cast<std::size_t>(state.range(0));
    ConstTable constants;
    const auto value = constants.find_or_add(int32_t{42});
    ObjectClass program(0, &constants);
    std::vector<uint8_t> bytecode;
    lpcvm::CodeGenerator generator(bytecode);
    generator.emit(F_CONST_I4, value);
    for (std::size_t access = 0; access < accesses; ++access)
        generator.emit(F_LOAD_LOCAL, 0).emit(F_STORE_LOCAL, 0).emit(F_POP_N, 1);
    generator.emit(F_RETURN, 1);

    BytecodeView code(bytecode.data(), bytecode.size(), &program);
    run_bytecode(state, code, accesses * 3 + 2, int32_t{42});
}
BENCHMARK(BM_LocalAccess)->Arg(64)->Arg(4096)->Arg(65536)->ArgName("accesses");

}
