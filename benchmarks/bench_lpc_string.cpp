// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <array>
#include <memory>
#include "bench_bytecode.hpp"
#include "code_generator.hpp"
#include "lpc_compiler.hpp"

namespace {
using namespace bm;

// These microbenchmarks call the same primitives as the store handlers. Each
// batch uses independent fixtures so first-store, COW, and intern-miss cases do
// not silently become reuse/hit cases after the first iteration.
enum class StringStore {
    SharedRef, BufferRef, SharedToBuffer, TempInit, TempUpgrade, TempReuse,
    TempCow, BufferInternHit, BufferInternMiss, TempInternHit, TempInternMiss,
    TempCopy, TempMerge
};

template<StringStore kind>
struct StringStoreFixture {
    ManagedRuntime fabric;
    lpcvm::DataStack stack;
    LpcBufferRef retained;
    SValue global;

    explicit StringStoreFixture(const std::string& bytes) {
        if constexpr (kind == StringStore::TempMerge) {
            stack.push_temp_string(bytes);
            stack.push_temp_string(bytes);
        } else if constexpr (kind == StringStore::BufferInternHit ||
                             kind == StringStore::BufferInternMiss ||
                             kind == StringStore::TempInternHit ||
                             kind == StringStore::TempInternMiss) {
            global = fabric.make_shared_string("old");
            if constexpr (kind == StringStore::BufferInternHit || kind == StringStore::TempInternHit)
                fabric.make_shared_string(bytes);
            if constexpr (kind == StringStore::BufferInternHit || kind == StringStore::BufferInternMiss) {
                auto source = fabric.make_buffer(bytes.size());
                source->data = bytes;
                stack.push(source);
            } else {
                stack.push_temp_string(bytes);
            }
        } else {
            if constexpr (kind == StringStore::SharedRef) {
                stack.push(fabric.make_shared_string("old"));
            } else if constexpr (kind == StringStore::TempUpgrade || kind == StringStore::TempCopy) {
                stack.push_temp_string(bytes);
            } else if constexpr (kind == StringStore::TempInit) {
                stack.push_n(1);
            } else {
                auto destination = fabric.make_buffer(bytes.size());
                destination->data.assign(bytes.size(), 'z');
                stack.push(destination);
                if constexpr (kind == StringStore::TempCow)
                    retained = destination;
            }
            if constexpr (kind == StringStore::SharedRef || kind == StringStore::SharedToBuffer) {
                stack.push(fabric.make_shared_string(bytes));
            } else if constexpr (kind == StringStore::BufferRef) {
                auto source = fabric.make_buffer(bytes.size());
                source->data = bytes;
                stack.push(source);
            } else {
                stack.push_temp_string(bytes);
            }
            // Reserve the extra slot used by assign_copy, outside timing.
            stack.push_n(1);
            stack.pop();
        }
    }

    void run() {
        if constexpr (kind == StringStore::TempMerge) {
            stack.concat_top_strings();
        } else if constexpr (kind == StringStore::BufferInternHit ||
                             kind == StringStore::BufferInternMiss ||
                             kind == StringStore::TempInternHit ||
                             kind == StringStore::TempInternMiss) {
            global = fabric.make_shared_string(stack.top_string());
            stack.pop(); // expression cleanup, as after F_STORE_GLOBAL
        } else {
            if constexpr (kind == StringStore::SharedRef || kind == StringStore::TempCopy)
                stack.assign_copy(0, 1);
            else
                stack.assign_value(0, fabric);
            stack.pop(); // expression cleanup, as after a local store
        }
        benchmark::ClobberMemory();
    }

    bool valid(const std::string& bytes) const {
        if constexpr (kind == StringStore::BufferInternHit ||
                      kind == StringStore::BufferInternMiss ||
                      kind == StringStore::TempInternHit ||
                      kind == StringStore::TempInternMiss) {
            return stack.empty() && std::get<LpcStringRef>(global)->view() == bytes;
        } else {
            if (stack.size() != 1)
                return false;
            if constexpr (kind == StringStore::TempMerge)
                return std::holds_alternative<TemporaryString>(stack[0]) && stack.string_at(0) == bytes + bytes;
            if constexpr (kind == StringStore::SharedRef || kind == StringStore::SharedToBuffer) {
                if (!std::holds_alternative<LpcStringRef>(stack[0])) return false;
            } else if constexpr (kind == StringStore::TempCopy) {
                if (!std::holds_alternative<TemporaryString>(stack[0])) return false;
            } else {
                if (!std::holds_alternative<LpcBufferRef>(stack[0])) return false;
            }
            if constexpr (kind == StringStore::TempCow) {
                if (retained->data != std::string(bytes.size(), 'z')) return false;
            }
            return stack.string_at(0) == bytes;
        }
    }
};

template<StringStore kind>
void string_assignment(benchmark::State& state) {
    constexpr std::size_t batch = 32;
    const std::string bytes(static_cast<std::size_t>(state.range(0)), 'a');
    std::array<std::unique_ptr<StringStoreFixture<kind>>, batch> fixtures;
    for (auto _ : state) {
        state.PauseTiming();
        for (auto& fixture : fixtures)
            fixture = std::make_unique<StringStoreFixture<kind>>(bytes);
        state.ResumeTiming();
        for (auto& fixture : fixtures)
            fixture->run();
        state.PauseTiming();
        bool valid = true;
        for (const auto& fixture : fixtures)
            valid = valid && fixture->valid(bytes);
        state.ResumeTiming();
        if (!valid) {
            state.SkipWithError("String assignment produced an unexpected value or representation");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations() * batch);
    state.counters["assignments/s"] = benchmark::Counter(
        batch, benchmark::Counter::kIsIterationInvariantRate);
}

#define STRING_ASSIGNMENT_BENCHMARK(name, kind) \
    void BM_StringAssign##name(benchmark::State& state) { string_assignment<StringStore::kind>(state); } \
    BENCHMARK(BM_StringAssign##name)->Arg(0)->Arg(16)->Arg(128)->Arg(4096)->ArgName("bytes")

STRING_ASSIGNMENT_BENCHMARK(SharedRef, SharedRef);
STRING_ASSIGNMENT_BENCHMARK(BufferRef, BufferRef);
STRING_ASSIGNMENT_BENCHMARK(SharedToBuffer, SharedToBuffer);
STRING_ASSIGNMENT_BENCHMARK(TempInit, TempInit);
STRING_ASSIGNMENT_BENCHMARK(TempUpgrade, TempUpgrade);
STRING_ASSIGNMENT_BENCHMARK(TempReuse, TempReuse);
STRING_ASSIGNMENT_BENCHMARK(TempCow, TempCow);
STRING_ASSIGNMENT_BENCHMARK(BufferInternHit, BufferInternHit);
STRING_ASSIGNMENT_BENCHMARK(BufferInternMiss, BufferInternMiss);
STRING_ASSIGNMENT_BENCHMARK(TempInternHit, TempInternHit);
STRING_ASSIGNMENT_BENCHMARK(TempInternMiss, TempInternMiss);
STRING_ASSIGNMENT_BENCHMARK(TempCopy, TempCopy);
STRING_ASSIGNMENT_BENCHMARK(TempMerge, TempMerge);
#undef STRING_ASSIGNMENT_BENCHMARK

// Keep trailing byte movement and stack-slot scanning independently variable.
void BM_StringTempRelease(benchmark::State& state) {
    constexpr std::size_t batch = 32;
    const std::string bytes(static_cast<std::size_t>(state.range(0)), 'a');
    const std::string trailing(static_cast<std::size_t>(state.range(1)), 'b');
    const auto slots = static_cast<std::size_t>(state.range(2));
    std::array<std::unique_ptr<lpcvm::DataStack>, batch> stacks;
    for (auto _ : state) {
        state.PauseTiming();
        for (auto& stack : stacks) {
            stack = std::make_unique<lpcvm::DataStack>();
            stack->push_temp_string(bytes);
            stack->push_n(slots);
            stack->push_temp_string(trailing);
        }
        state.ResumeTiming();
        for (auto& stack : stacks) {
            stack->assign(0, int32_t{0});
            benchmark::ClobberMemory();
        }
        state.PauseTiming();
        bool valid = true;
        for (const auto& stack : stacks)
            valid = valid && stack->size() == slots + 2 &&
                std::get<int32_t>((*stack)[0]) == 0 && stack->top_string() == trailing;
        state.ResumeTiming();
        if (!valid) {
            state.SkipWithError("Temporary release corrupted the remaining stack");
            break;
        }
    }
    state.SetItemsProcessed(state.iterations() * batch);
}
BENCHMARK(BM_StringTempRelease)
    ->ArgsProduct({{0, 16, 4096}, {0, 16, 4096}, {0, 64, 4096}})
    ->ArgNames({"released_bytes", "trailing_bytes", "other_slots"});

void string_local_concat(benchmark::State& state, bool accumulator, bool buffer = false) {
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
    options.use_local_string_buffer = buffer;
    auto program = compiler.generate_code(options);
    lpcvm::Execution code(std::make_shared<ManagedRuntime>());
    if (!program || !program->lookup_method("run", &code)) {
        state.SkipWithError("Could not generate string benchmark bytecode");
        return;
    }
    std::size_t logical_instructions = 0;
    std::size_t accumulator_begins = 0;
    std::size_t buffer_stores = 0;
    std::size_t buffer_finishes = 0;
    for (std::size_t offset = 0; offset < code.length(); offset += 2) {
        if (code.start_[offset] != F_EXTENDED_ARG)
            ++logical_instructions;
        if (code.start_[offset] == F_STR_BEGIN)
            ++accumulator_begins;
        if (code.start_[offset] == F_STORE_LOCAL_BUFFER)
            ++buffer_stores;
        if (code.start_[offset] == F_STR_FINISH_LOCAL)
            ++buffer_finishes;
    }
    // left and right are stored; each result assignment is stored or finished into its buffer
    if (accumulator_begins != (accumulator ? concatenations : 0) ||
        buffer_finishes != (accumulator && buffer ? concatenations : 0) ||
        buffer_stores + buffer_finishes != (buffer ? concatenations + 2 : 0)) {
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

void BM_StringLocalConcatBuffer(benchmark::State& state) {
    string_local_concat(state, false, true);
}
BENCHMARK(BM_StringLocalConcatBuffer)
    ->ArgsProduct({{64, 4096}, {0, 16, 128, 4096}, {2, 8, 32}})
    ->ArgNames({"assignments", "operand_bytes", "terms"});

void BM_StringLocalConcatAccumulatorBuffer(benchmark::State& state) {
    string_local_concat(state, true, true);
}
BENCHMARK(BM_StringLocalConcatAccumulatorBuffer)
    ->ArgsProduct({{64, 4096}, {0, 16, 128, 4096}, {2, 8, 32}})
    ->ArgNames({"assignments", "operand_bytes", "terms"});

} // anonymous namespace
