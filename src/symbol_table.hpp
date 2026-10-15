// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include "const_pool.hpp"

/// @brief Enumerates the basic data types available in the LPC language.
enum LpcDataType {
    T_VOID = 0, // for function return type
    T_INT,
    T_FLOAT,
    T_DOUBLE,
    T_STRING,
    T_MAPPING,
    T_OBJECT,
    T_MIXED
};

/// @brief Represents the signature of a symbol, including its data type and various attributes.
typedef union symbol_signature_u {
    uint32_t value;
    struct {
        uint32_t data_type : 4;
        uint32_t is_array : 1;
        uint32_t is_const : 1; // indicates if the symbol is a compile-time constant
        uint32_t is_static : 1; // storage class specifier
        uint32_t is_public : 1; // visibility specifier
        uint32_t is_nomask : 1; // indicates if the symbol cannot be overridden by derived classes
        uint32_t reserved : 22; // padding to make the struct 32 bits
    } b;
} SymbolSignature;

/**
 * \brief Represents a symbol table for managing variable and function symbols within a scope.
 *
 * The symbol table maintains a mapping from mangled symbol name to their corresponding
 * entries and allows efficient lookup and insertion of symbols.
 *
 * Supports nested scopes through the enclosing_scope_ pointer.
 * The symbol table contains all compile-time symbols including constants and private members.
 *
 * For runtime symbol bindings, see VirtualTable which allows index-based access (for static
 * bindings) and name-based dynamic dispatch (for virtual methods, aka. call_other).
 */
class SymbolTable: public std::enable_shared_from_this<SymbolTable> {
public:
    struct Entry {
        std::string name;
        SymbolSignature signature;
        std::optional<ConstTable::ValueType> value; // compile-time constant value if the symbol is constant
        Entry(std::string name, SymbolSignature signature)
            : name(std::move(name)), signature(signature) {}
        Entry() = default;
    };

    SymbolTable (std::shared_ptr<SymbolTable> enclosing_scope)
        : enclosing_scope_ (std::move(enclosing_scope)) {}

    std::shared_ptr<SymbolTable> new_subscope () { return std::make_shared<SymbolTable>(shared_from_this()); }
    std::shared_ptr<SymbolTable> get_enclosing_scope() const { return enclosing_scope_; }

    /**
     * \brief Looks up a symbol entry by its key in the current scope and enclosing scopes.
     * \param mangled_name The mangled name of the symbol to look up.
     * \return A pointer to the corresponding entry if found, or nullptr if not found.
     */
    std::shared_ptr<Entry> lookup (std::string_view mangled_name) const;

    /**
     * \brief Inserts a new symbol entry into the current scope of the symbol table.
     * \param mangled_name The mangled name of the symbol to insert.
     * \param entry A shared pointer to the Entry to be inserted.
     */
    void emplace (std::string_view mangled_name, const std::shared_ptr<Entry>& entry);

    std::size_t size() const { return symbols_.size(); }

private:
    std::shared_ptr<SymbolTable> enclosing_scope_;
    std::unordered_map<std::string, std::shared_ptr<Entry>> symbols_;
};
