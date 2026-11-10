// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"

#include "bench_bytecode.hpp"
#include "code_generator.hpp"

namespace {

using namespace bm;

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

} // anonymous namespace
