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
    VirtualTable() = default;
    ~VirtualTable() = default;

    struct binding_s {
        SymbolSignature signature;
        int32_t base_index;
        int32_t bytecode_offset;
        uint32_t bytecode_length;
        // we don't need to store number of local variables, as it is created via opcode F_PUSH_LOCALS at runtime.
    };

    /**
     * \brief Represents a method binding in the virtual table.
     *
     * The first element of the pair is the base class index (-1 for current class),
     * and the second element is the method bytecode offset (-1 for forward declaration only).
     */
    using Binding = binding_s;

    const std::vector<std::pair<std::string_view, Binding>>& bindings() const {
        return bindings_;
    }

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

    /**
     * \brief Adds a new method binding to the virtual table.
     *
     * \param name The name of the method to add. The name must be interned
     * (i.e., stored in a constant pool that outlives the virtual table).
     * \param binding The binding information for the method.
     */
    void emplace (const std::string_view name, const Binding& binding);

    std::vector<std::pair<std::string_view, Binding>> bindings_; // sorted by method name
};
