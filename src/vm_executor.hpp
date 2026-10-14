// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include "const_pool.hpp"

class BytecodeView {
public:
    BytecodeView(const uint8_t* bytecode, std::size_t size, const ConstPool* const_pool)
        : start_(bytecode), end_(bytecode + size), const_pool_(const_pool) {}

    void execute();

private:
    const uint8_t* start_;
    const uint8_t* end_;
    const ConstPool* const_pool_;
};
