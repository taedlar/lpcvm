// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include "const_pool.hpp"
#include "location.hh"
#include "lpc_types.hpp"
#include "vm_sandbox.hpp"

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
        std::shared_ptr<ConstTable> consts;

        ConstPool::IndexType current_file; // index of the current source filename in the constant pool
        ConstPool::IndexType current_func; // index of the current function name in the constant pool
        uint32_t next_func_idx; // index for the next function
        uint32_t next_global;
        uint32_t next_local;

        std::string current_string;
        std::string raw_string_delimiter;
        yy::location location;
        yy::position string_begin;
    
        Context(): current_file(ConstPool::INVALID_INDEX), current_func(ConstPool::INVALID_INDEX), next_func_idx(0), next_global(0), next_local(0) {}
        ~Context() = default;
    };

    // compiler messages (errors, warnings, etc.)
    struct Message {
        int lineno;
        int column;
        std::string text;
    };

    void set_filename(const std::string_view fn) { filename_ = fn; }

    bool parse() noexcept;
    bool parse (const std::string& source) noexcept;
    std::shared_ptr<ObjectClass> generate_code() noexcept;

    const std::vector<Message>& messages() const { return messages_; }

    const SymbolTable* get_globals() const { return globals_.get(); }
    const ConstTable* get_consts() const { return consts_.get(); }
    const ASTNode* get_ast() const { return ast_.get(); }

private:
    struct internal_s;
    internal_s* impl_;

    FILE* source_file_;
    std::string filename_;  // for yylloc
    std::vector<Message> messages_;

    std::shared_ptr<ConstTable> consts_;
    std::shared_ptr<SymbolTable> globals_;
    std::shared_ptr<ASTNode> ast_;

    bool init();
};
