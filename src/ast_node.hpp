// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <memory>
#include <utility>
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
        FunctionDef,
        ParameterList,
        Block,
        NewLocal,
        Assign,
        UnaryOp,
        BinaryOp,
        TernaryOp,
        Expression,
        Constant,
        Variable,
        FunctionCall,
        // Add other AST node types here
    };

    AstNode(Type type) : type_(type) {}
    virtual ~AstNode() = default;

    inline AstNode& add_child(std::shared_ptr<AstNode> child) {
        if (child)
            children_.push_back(std::move(child));
        return *this;
    }

    Type get_type() const { return type_; }

    virtual void generate_code(std::vector<uint8_t>& bytecode) = 0;

private:
    Type type_ = Type::Unknown;

protected:
    std::vector<std::shared_ptr<AstNode>> children_;
};
