// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include "lpc_types.hpp"

struct ASTNode;
class ConstTable;
class SymbolTable;
class RuntimeClass;

class LpcCompiler {
public:
    LpcCompiler();
    ~LpcCompiler();

    // context data used by the lexer and parser
    struct Context {
        std::shared_ptr<ASTNode> root;
        std::shared_ptr<SymbolTable> current_scope;
        ConstTable* consts;

        std::string current_string;
        std::string raw_string_delimiter;
    };

    // compiler messages (errors, warnings, etc.)
    struct Message {
        int lineno;
        int column;
        std::string text;
    };

    ASTNode* frontend (const std::string& source) noexcept;

    const std::vector<Message>& messages() const { return messages_; }

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
    std::vector<Message> messages_;

    std::unique_ptr<ConstTable> consts_;
    std::shared_ptr<SymbolTable> globals_;
    std::shared_ptr<ASTNode> ast_;
};
