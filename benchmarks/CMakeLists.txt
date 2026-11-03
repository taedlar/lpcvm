# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

add_executable(bench_bytecode bench_bytecode.cpp)
target_link_libraries(bench_bytecode PRIVATE lpcvm_obj spdlog benchmark::benchmark_main)

add_executable(bench_macro_expansion bench_macro_expansion.cpp)
target_link_libraries(bench_macro_expansion PRIVATE lpcvm_obj spdlog benchmark::benchmark_main)
