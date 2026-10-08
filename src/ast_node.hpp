// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <memory>
#include <vector>

/**
 * \brief Abstract base class for all AST nodes.
 *
 * An AST (Abstract Syntax Tree) node represents a single element in the hierarchical structure
 * of source code.
 */
class AstNode {
public:
    enum class Type {
        Unknown,
        Program,
        // Add other AST node types here
    };

    AstNode(int line, Type type) : line_number_(line), type_(type) {}
    virtual ~AstNode() = default;

    inline AstNode& add_child(std::shared_ptr<AstNode> child) {
        children_.push_back(std::move(child));
        return *this;
    }

    Type get_type() const { return type_; }
    int get_line_number() const { return line_number_; }

    virtual void generate_code(std::vector<uint8_t>& bytecode) = 0;

private:
    int line_number_ = 0;
    Type type_ = Type::Unknown;

protected:
    std::vector<std::shared_ptr<AstNode>> children_;
};
