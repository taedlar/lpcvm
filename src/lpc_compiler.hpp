// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
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
        std::string current_string;
        std::string raw_string_delimiter;
    };

    void compile(const std::string& source);

private:
    class Impl;
    Impl* pimpl;
};
