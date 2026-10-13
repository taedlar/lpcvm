// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <unordered_map>
#include <optional>
#include <vector>
#include "const_pool.hpp"

class RuntimeClass {
public:
    RuntimeClass() = default;
    ~RuntimeClass() = default;

    using MemberIndexType = ConstPool::IndexType; // index to member name in the constant pool

private:
    std::vector<uint8_t> bytecode_;
    std::optional<ConstPool> const_pool_;
    MemberIndexType num_members_; // number of members in the runtime class (not including inherited members)
    std::vector<std::shared_ptr<RuntimeClass>> inherits_; // inherited base classes

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
