// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include "const_pool.hpp"

class ClassBlueprint {
public:
    ClassBlueprint() = default;
    ~ClassBlueprint() = default;

    using BaseIndexType = ConstPool::IndexType; // index to inherited blueprint name in the constant pool
    using MemberIndexType = ConstPool::IndexType; // index to member name in the constant pool

private:
    std::vector<uint8_t> bytecode_;
    std::optional<ConstPool> const_pool_;
};
