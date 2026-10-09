// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "symbol_table.hpp"
#include "ast_node.hpp"

class AstFunctionNode : public AstNode {
public:
    AstFunctionNode (int line, std::string name, SymbolSignature signature)
        : AstNode(line, Type::Function), name_(std::move(name)), signature_(signature) {}
    void generate_code(std::vector<uint8_t>& code) override {}

private:
    std::string name_;
    SymbolSignature signature_;
};
