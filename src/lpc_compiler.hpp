// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "lpc_types.hpp"

struct ASTNode;
class ConstTable;
class SymbolTable;
class ObjectClass;

class LpcCompiler {
public:
    LpcCompiler(FILE* source_file = nullptr);
    ~LpcCompiler();

    // context data used by the lexer and parser
    struct Context {
        std::shared_ptr<ASTNode> root;
        std::shared_ptr<SymbolTable> current_scope;
        ConstTable* consts;

        std::string current_string;
        std::string raw_string_delimiter;
        uint32_t next_global;
        uint32_t next_local;
    
        Context(): consts(nullptr), next_global(0), next_local(0) {}
        ~Context() = default;
    };

    // compiler messages (errors, warnings, etc.)
    struct Message {
        int lineno;
        int column;
        std::string text;
    };

    bool parse() noexcept;
    bool parse (const std::string& source) noexcept;
    std::shared_ptr<ObjectClass> generate_code() noexcept;

    const std::vector<Message>& messages() const { return messages_; }

    const SymbolTable* get_globals() const { return globals_.get(); }
    const ConstTable* get_consts() const { return consts_.get(); }
    const ASTNode* get_ast() const { return ast_.get(); }

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
    FILE* source_file_;
    std::vector<Message> messages_;

    std::shared_ptr<ConstTable> consts_;
    std::shared_ptr<SymbolTable> globals_;
    std::shared_ptr<ASTNode> ast_;
};
