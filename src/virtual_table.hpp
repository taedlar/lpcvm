// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include "symbol_table.hpp"
#include "const_pool.hpp"

/**
 * \brief Represents the virtual table for a runtime class, managing method
 * bindings and enabling dynamic dispatch.
 *
 * The virtual table contains only non-private method bindings.
 * Private methods, variables, and constants are resolved to indices at compile time.
 */
class VirtualTable {
public:
    VirtualTable(std::shared_ptr<ConstPool> name_pool) : name_pool_(std::move(name_pool)) {}
    ~VirtualTable() = default;

    /**
     * \brief Represents a method binding in the virtual table.
     */
    struct binding_s {
        SymbolSignature signature;
        int32_t inheriting;
        int32_t bytecode_offset;
        uint32_t bytecode_length;
        // we don't need to store number of local variables, as it is created via opcode F_PUSH_LOCALS at runtime.
    };

    using Binding = binding_s;

    /**
     * \brief Looks up a method binding by its name (binary search).
     *
     * \param name The name of the method to look up.
     * \param out_binding A reference to a Binding object where the result will be stored if found.
     * \return true if the method is found, false otherwise.
     */
    bool lookup (const std::string_view name, Binding& out_binding) const;

private:
    friend class SymbolTable;
    friend class LpcCompiler;

    /**
     * \brief Adds a new method binding to the virtual table.
     *
     * \param name_index The index of the method name in the constant pool.
     * \param binding The binding information for the method.
     */
    void emplace (ConstPool::IndexType name_index, const Binding& binding);

    std::shared_ptr<ConstPool> name_pool_;

    /**
     * \brief The list of method bindings in the virtual table, sorted by
     * method name. The method name must be interned in the constant pool.
     */
    std::vector<std::pair<std::string_view, Binding>> bindings_;
};
