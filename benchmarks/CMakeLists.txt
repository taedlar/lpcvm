# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

add_executable(run_bench
    bench_bytecode.hpp
    bench_bytecode.cpp
    bench_vm_stack.cpp
    bench_lpc_string.cpp
    bench_macro_expansion.cpp
)
target_link_libraries(run_bench PRIVATE lpcvm_obj spdlog benchmark::benchmark_main)
