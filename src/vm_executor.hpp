// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>

class BytecodeView {
public:
    BytecodeView(const uint8_t* bytecode, std::size_t size)
        : bytecode_(bytecode), size_(size) {}

    void execute();

private:
    const uint8_t* bytecode_;
    std::size_t size_;
};
