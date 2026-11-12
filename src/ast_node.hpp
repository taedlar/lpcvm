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
#include "object_class.hpp"

/**
 * @brief Represents a 1-based source position (zero means unknown).
 */
struct ScannerLocation {
    ConstPool::IndexType current_file; // index of the current source filename in the constant pool
    ConstPool::IndexType current_func; // index of the current function name in the constant pool
    int32_t lineno;
    int32_t column;
};

struct ASTNode;

/**
 * Abstract Syntax Tree (AST) node definitions for the LPC virtual machine.
 * Each struct represents a different kind of AST node.
 *
 * The variant part of ASTNode stores the actual node data (with 8 bytes index)
 * and:
 * - three std::unique_ptr (8 bytes each)
 * - a std::shared_ptr (16 bytes) and a std::unique_ptr (8 bytes)
 * - a std::vector (24 bytes)
 *
 * The size of ASTNode is determined by the largest variant alternative.
 * In current implementation, we keep the variant part within 32 bytes so the
 * total size of ASTNode is 48 bytes (32 bytes variant data plus 16 bytes
 * lexial scanner location record).
 *
 * Use std::move when passing ASTNode. Assignment from a mutable ASTNode
 * also transfers ownership to support Bison's default action ($$ = $1).
 */
namespace AST {
    struct Program {
        std::vector<ASTNode> defs; // *(FunctionDecl | FunctionDef | Inheritance)
    };
    struct Inheritance {
        std::shared_ptr<SymbolTable::Entry> entry; // a pseudo entry mangled from the file path
    };
    struct FunctionDecl {
        std::shared_ptr<SymbolTable::Entry> entry;
        std::unique_ptr<ASTNode> param_list; // ParameterList
    };
    struct ParameterList {
        std::vector<SymbolSignature> parameters;
    };
    struct FunctionDef {
        std::unique_ptr<ASTNode> decl; // FunctionDecl
        std::unique_ptr<ASTNode> body; // Block
    };
    struct Block {
        uint32_t num_locals; // for block scope
        std::unique_ptr<ASTNode> stmts;
    };
    struct Cond {
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<ASTNode> body;
        std::unique_ptr<ASTNode> else_body; // optional else body
    };
    struct PreTestedLoop {
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<ASTNode> step;
        std::unique_ptr<ASTNode> body;
    };
    struct PostTestedLoop {
        std::unique_ptr<ASTNode> body;
        std::unique_ptr<ASTNode> condition;
    };
    struct IterativeLoop {
        std::unique_ptr<ASTNode> iter;
        std::unique_ptr<ASTNode> collection;
        std::unique_ptr<ASTNode> body;
    };
    struct Return {
        std::unique_ptr<ASTNode> expr;
    };
    struct Break {
    };
    struct Continue {
    };
    struct Switch {
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<ASTNode> body;
    };
    struct TryCatch {
        std::unique_ptr<ASTNode> try_block;
        std::unique_ptr<ASTNode> catch_param; // ParameterList to bind the caught exception
        std::unique_ptr<ASTNode> catch_block;
    };
    struct Assign {
        int arg; // namespace oparg::AssignOpArg
        std::unique_ptr<ASTNode> lhs;
        std::unique_ptr<ASTNode> rhs;
    };
    struct UnaryOp {
        int arg; // namespace oparg::UnaryOpArg
        std::unique_ptr<ASTNode> operand;
    };
    struct BinaryOp {
        int arg; // namespace oparg::BinaryOpArg
        std::unique_ptr<ASTNode> lhs;
        std::unique_ptr<ASTNode> rhs;
    };
    struct TernaryOp {
        std::unique_ptr<ASTNode> condition;
        std::unique_ptr<ASTNode> true_expr;
        std::unique_ptr<ASTNode> false_expr;
    };
    struct List {
        std::vector<ASTNode> elts;
    };
    struct FunctionCall {
        std::shared_ptr<SymbolTable::Entry> entry; // the function being called
        std::unique_ptr<ASTNode> arguments; // List
    };
    struct NumericConst {
        ConstTable::NumericType value;
        ConstPool::IndexType index;
    };
    struct StringLiteral {
        ConstPool::IndexType idx;
        StringLiteral(const std::string& str, ConstTable& const_table)
            : idx(const_table.find_or_add(str)) {}
    };
    struct Variable {
        std::shared_ptr<SymbolTable::Entry> entry;
    };
    struct Self {
    };
    struct New {
        std::unique_ptr<ASTNode> class_name; // str_literal
        std::unique_ptr<ASTNode> arg_list; // List
    };
}

struct ASTNode {
    using Storage = std::variant<
        std::monostate,
        AST::Program,
        AST::Inheritance,
        AST::FunctionDef,
        AST::FunctionDecl,
        AST::ParameterList,
        AST::Block,
        AST::Cond,
        AST::PreTestedLoop,
        AST::PostTestedLoop,
        AST::IterativeLoop,
        AST::Return,
        AST::Break,
        AST::Continue,
        AST::Switch,
        AST::TryCatch,
        AST::Assign,
        AST::UnaryOp,
        AST::BinaryOp,
        AST::TernaryOp,
        AST::List,
        AST::FunctionCall,
        AST::NumericConst,
        AST::StringLiteral,
        AST::Variable,
        AST::Self,
        AST::New
    >;

    ASTNode() = default;
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    // Bison's default rule action assigns from a mutable semantic value.
    ASTNode& operator=(ASTNode& value) noexcept {
        return *this = std::move(value);
    }
    ASTNode(ASTNode&&) = default;
    ASTNode& operator=(ASTNode&&) = default;

    template<typename T>
    ASTNode(T&& value, ScannerLocation source = {}) : node(std::forward<T>(value)), src(source) {}

    Storage node;
    ScannerLocation src{};
};
