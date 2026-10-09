// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

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
 * \brief Represents an entry in the symbol table, encapsulating the symbol's name and signature.
 */
class SymbolEntry {
public:
    SymbolEntry (std::string name, SymbolSignature signature)
        : name_ (std::move(name)), signature_ (signature) {}

    const std::string& get_name() const { return name_; }
    SymbolSignature get_signature() const { return signature_; }

    using ValueType = std::variant<size_t, long, double, float, std::string>;

    /// @brief Assigns a value to the symbol. For example: runtime index of variables or methods.
    /// @param value The value to assign to the symbol.
    void assign_value (ValueType value) {
        value_ = std::move(value);
    }

    /// @brief Assigns a declared constant value to the symbol and marks it as constant.
    void assign_constant (ValueType value) {
        value_ = std::move(value);
        signature_.b.is_const = 1;
    }

private:
    std::string name_;
    SymbolSignature signature_;
    std::optional<ValueType> value_; // compile-time constant value if the symbol is constant
};

/**
 * \brief Represents a symbol table for managing variable and function symbols within a scope.
 *
 * The symbol table maintains a mapping from mangled symbol name to their corresponding
 * entries and allows efficient lookup and insertion of symbols.
 *
 * Supports nested scopes through the enclosing_scope_ pointer.
 */
class SymbolTable {
public:
    SymbolTable (std::shared_ptr<SymbolTable> enclosing_scope)
        : enclosing_scope_ (std::move(enclosing_scope)) {}

    /**
     * \brief Looks up a symbol entry by its key in the current scope and enclosing scopes.
     * \param mangled_name The mangled name of the symbol to look up.
     * \return A pointer to the corresponding SymbolEntry if found, or nullptr if not found.
     */
    SymbolEntry* lookup (const std::string& mangled_name) const;
    void insert (const std::string& mangled_name, const std::shared_ptr<SymbolEntry>& entry);

private:
    std::shared_ptr<SymbolTable> enclosing_scope_;
    std::unordered_map<std::string, std::shared_ptr<SymbolEntry>> symbols_;
};
