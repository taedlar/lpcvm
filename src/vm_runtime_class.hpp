// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "const_pool.hpp"
#include "virtual_table.hpp"

class RuntimeClass {
public:
    RuntimeClass() = default;
    ~RuntimeClass() = default;

    using MemberIndexType = ConstPool::IndexType; // index to member name in the constant pool

    void inherit_from(std::shared_ptr<RuntimeClass> parent);

    /// @brief Returns the total number of instance variables for this runtime class,
    /// including those inherited from parent classes.
    /// @return The total number of instance variables.
    std::size_t num_instance_variables() const {
        std::size_t total = num_variables_;
        for (const auto& parent : inherits_) {
            total += parent->num_instance_variables();
        }
        return total;
    }

    bool lookup_method (const std::string_view name, VirtualTable::Binding& out_binding) const {
        return virtual_table_ ? virtual_table_->lookup(name, out_binding) : false;
    }

    /**
     * \brief Registers a runtime class in the global registry (replaces any
     * existing entry with the same path).
     *
     * \param path The path of the class in the mudlib (e.g., "domain/area/object")
     * without the file extension.
     * \param runtime_class A shared pointer to the runtime class to be registered.
     */
    static void register_class (const std::string& path, std::shared_ptr<RuntimeClass> runtime_class);

    /**
     * \brief Retrieves a runtime class from the global registry by its path.
     *
     * \param path The path of the class in the mudlib (e.g., "domain/area/object")
     * without the file extension.
     * \return A shared pointer to the runtime class if found, or nullptr if not found.
     */
    static std::shared_ptr<RuntimeClass> get_class(const std::string& path);

private:
    std::vector<uint8_t> bytecode_;
    std::vector<std::shared_ptr<RuntimeClass>> inherits_;
    std::unique_ptr<ConstPool> const_pool_;
    std::unique_ptr<VirtualTable> virtual_table_;
    MemberIndexType num_variables_; // (not including inherited variables)

    /**
     * \brief A global registry of all in-memory runtime classes, keyed
     * by their path in the mudlib (e.g., "domain/area/object") without the
     * file extension.
     *
     * Each node in the registry may hold references to their parent
     * runtime classes (i.e., inherited classes). If a class is hot-reloaded,
     * the registry should be updated to reflect the changes. Existing
     * references to the old versions of the classes may keep pointing to
     * outdated definitions.
     *
     * \note This design allows for dynamic updates to the class hierarchy
     * while maintaining backward compatibility with existing instances.
     */
    static std::unordered_map<std::string, std::shared_ptr<RuntimeClass>> registry_;
};
