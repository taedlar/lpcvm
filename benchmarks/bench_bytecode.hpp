// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <benchmark/benchmark.h>
#include "vm_executor.hpp"

namespace bm {

void run_bytecode(benchmark::State& state, lpcvm::Execution& code,
    std::size_t logical_instructions,
    std::variant<std::monostate, int32_t, std::string_view> expected = std::monostate{});

} // namespace bm
