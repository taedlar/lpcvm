// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

class LpcObject;
class LpcMapping;
class LpcArray;

using ArenaStringView = std::pair<uint32_t, uint32_t>; // first: offset, second: length in the arena
using LpcObjectRef = std::size_t;
using LpcMappingRef = std::size_t;
using LpcArrayRef = std::size_t;

class SValue: public std::variant<
    std::monostate,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t,
    float,
    double,
    ArenaStringView,
    LpcObjectRef,
    LpcMappingRef,
    LpcArrayRef
> {
public:
    SValue() = default;

    static std::vector<std::weak_ptr<LpcObject>> all_object_refs; // strong references hold by LpcObject registry
    static std::vector<std::shared_ptr<LpcMapping>> all_mapping_refs;
    static std::vector<std::shared_ptr<LpcArray>> all_array_refs;

    static std::vector<LpcObjectRef> free_object_refs;
    static std::vector<LpcMappingRef> free_mapping_refs;
    static std::vector<LpcArrayRef> free_array_refs;
};
