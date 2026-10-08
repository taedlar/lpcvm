// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstddef>
#include <memory>
#include <string>

#undef YY_DECL
#define YY_DECL yy::LpcParser::symbol_type yylex (yyscan_t yyscanner)
#define YY_EXTRA_TYPE LpcCompiler::Context*

class AstNode;

class LpcCompiler {
public:
    LpcCompiler();
    ~LpcCompiler();

    // context data used by the lexer and parser
    struct Context {
        std::shared_ptr<AstNode> prog;
        std::string current_string;
        std::string raw_string_delimiter;
    };

    void compile (const std::string& source);
    int current_lineno() const;
    int current_column() const;

private:
    class Impl;
    Impl* pimpl;
};
