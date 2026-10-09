// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstddef>
#include <memory>
#include <string>

class AstNode;
class ConstTable;
class SymbolTable;

class LpcCompiler {
public:
    LpcCompiler();
    ~LpcCompiler();

    // context data used by the lexer and parser
    struct Context {
        std::shared_ptr<AstNode> prog;
        std::shared_ptr<SymbolTable> current_scope;
        ConstTable* consts;

        int current_lineno;
        int current_column;
        std::string current_string;
        std::string raw_string_delimiter;
    };

    bool compile (const std::string& source);
    int current_lineno() const;
    int current_column() const;

private:
    class Impl;
    Impl* pimpl_;
};
