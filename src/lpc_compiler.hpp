// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <string>

class LpcCompiler {
public:
    LpcCompiler();
    ~LpcCompiler();

    // context data used by the lexer and parser
    struct Context {
        std::string current_string;
        std::string raw_string_delimiter;
    };

    void compile(const std::string& source);

private:
    class Impl;
    Impl* pimpl;
};
