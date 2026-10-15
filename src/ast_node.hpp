// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <memory>
#include <utility>
#include <variant>
#include <vector>
#include "const_pool.hpp"
#include "symbol_table.hpp"

class RuntimeClass;

struct ASTNode;

namespace AST {
    struct Program {
        std::vector<ASTNode> defs;
    };
    struct Inheritance {
        ConstPool::IndexType idx_name;
        std::shared_ptr<RuntimeClass> runtime_class;
    };
    struct FunctionDef {
        std::shared_ptr<SymbolTable::Entry> entry;
        std::unique_ptr<ASTNode> param_list;
        std::unique_ptr<ASTNode> body;
    };
    struct ParameterList {
        std::vector<ASTNode> parameters;
    };
    struct Block {
        std::vector<ASTNode> statements;
    };
    struct VariableDecl {
        std::shared_ptr<SymbolTable::Entry> entry;
        std::unique_ptr<ASTNode> init;
    };
    struct VariableDeclList {
        std::vector<ASTNode> declarations;
    };
    struct Assign {
        std::unique_ptr<ASTNode> lhs;
        std::unique_ptr<ASTNode> rhs;
    };
    struct UnaryOp {
        std::unique_ptr<ASTNode> operand;
    };
    struct BinaryOp {
        std::unique_ptr<ASTNode> lhs;
        std::unique_ptr<ASTNode> rhs;
    };
    struct TernaryOp {
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<ASTNode> true_expr;
        std::unique_ptr<ASTNode> false_expr;
    };
    struct FunctionCall {
        std::vector<ASTNode> arguments;
    };
    struct Constant {
        ConstTable::NumericType value;
    };
    struct StringLiteral {
        ConstPool::IndexType idx;
        StringLiteral(const std::string& str, ConstTable& const_table)
            : idx(const_table.find_or_add(str)) {}
    };
    struct Variable {
        std::shared_ptr<SymbolTable::Entry> entry;
    };
}

struct ASTNode {
    using Storage = std::variant<
        std::monostate,
        AST::Program,
        AST::Inheritance,
        AST::VariableDecl,
        AST::VariableDeclList,
        AST::FunctionDef,
        AST::ParameterList,
        AST::Block,
        AST::Assign,
        AST::UnaryOp,
        AST::BinaryOp,
        AST::TernaryOp,
        AST::FunctionCall,
        AST::Constant,
        AST::StringLiteral,
        AST::Variable
    >;

    ASTNode() = default;
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    ASTNode(ASTNode&&) = default;
    ASTNode& operator=(ASTNode&&) = default;

    template<typename T>
    ASTNode(T&& value) : node(std::forward<T>(value)) {}

    Storage node;
};

// some utility for std::visit with multiple lambdas
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
