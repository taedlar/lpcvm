// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstddef>
#include <string>

#undef YY_DECL
#define YY_DECL yy::LpcParser::symbol_type yylex (yyscan_t yyscanner)
#define YY_EXTRA_TYPE LpcCompiler::Context*

class LpcCompiler {
public:
    LpcCompiler();
    ~LpcCompiler();

    // context data used by the lexer and parser
    struct Context {
        // One-based source line at the lexer's current position.
        std::size_t line_number = 1;
        std::string current_string;
        std::string raw_string_delimiter;
    };

    void compile(const std::string& source);
    std::size_t get_line_number() const;

private:
    class Impl;
    Impl* pimpl;
};
