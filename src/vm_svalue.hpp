// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>
#include <mutex>

class LpcObject;
class LpcMapping;
class LpcArray;

using ArenaStringView = std::pair<uint32_t, uint32_t>; // first: offset, second: length in the arena

template <typename T>
struct LpcRef {
    std::size_t index;

    constexpr LpcRef(std::size_t index) : index(index) {}
};

using LpcObjectRef = LpcRef<LpcObject>;
using LpcMappingRef = LpcRef<LpcMapping>;
using LpcArrayRef = LpcRef<LpcArray>;

template <typename Variant, typename T, std::size_t I = 0>
constexpr std::size_t variant_index() {
    if constexpr (I >= std::variant_size_v<Variant>) {
        return std::variant_size_v<Variant>; // Type not found
    } else {
        if constexpr (std::is_same_v<std::variant_alternative_t<I, Variant>, T>) {
            return I;
        } else {
            return variant_index<Variant, T, I + 1>();
        }
    }
}

using SValueVariant = std::variant<
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
>;

class SValue: public SValueVariant {
public:
    SValue() = default;

    static std::mutex object_refs_mutex;
    static std::mutex mapping_refs_mutex;
    static std::mutex array_refs_mutex;

    static std::vector<std::weak_ptr<LpcObject>> all_object_refs; // strong references hold by LpcObject registry
    static std::vector<std::shared_ptr<LpcMapping>> all_mapping_refs;
    static std::vector<std::shared_ptr<LpcArray>> all_array_refs;

    static std::vector<LpcObjectRef> free_object_refs;
    static std::vector<LpcMappingRef> free_mapping_refs;
    static std::vector<LpcArrayRef> free_array_refs;

    // Resolve reference alternatives at compile time from their distinct types.
    SValue(const std::shared_ptr<LpcObject>& obj) : SValue() {
        std::lock_guard<std::mutex> lock(object_refs_mutex);
        if (!free_object_refs.empty()) {
            LpcObjectRef ref = free_object_refs.back();
            free_object_refs.pop_back();
            all_object_refs[ref.index] = obj;
            emplace<variant_index<SValueVariant, LpcObjectRef>()>(ref);
        } else {
            LpcObjectRef ref = all_object_refs.size();
            all_object_refs.push_back(obj);
            emplace<variant_index<SValueVariant, LpcObjectRef>()>(ref);
        }
    }
    
    SValue(const std::shared_ptr<LpcMapping>& mapping) : SValue() {
        std::lock_guard<std::mutex> lock(mapping_refs_mutex);
        if (!free_mapping_refs.empty()) {
            LpcMappingRef ref = free_mapping_refs.back();
            free_mapping_refs.pop_back();
            all_mapping_refs[ref.index] = mapping;
            emplace<variant_index<SValueVariant, LpcMappingRef>()>(ref);
        } else {
            LpcMappingRef ref = all_mapping_refs.size();
            all_mapping_refs.push_back(mapping);
            emplace<variant_index<SValueVariant, LpcMappingRef>()>(ref);
        }
    }

    SValue(const std::shared_ptr<LpcArray>& array) : SValue() {
        std::lock_guard<std::mutex> lock(array_refs_mutex);
        if (!free_array_refs.empty()) {
            LpcArrayRef ref = free_array_refs.back();
            free_array_refs.pop_back();
            all_array_refs[ref.index] = array;
            emplace<variant_index<SValueVariant, LpcArrayRef>()>(ref);
        } else {
            LpcArrayRef ref = all_array_refs.size();
            all_array_refs.push_back(array);
            emplace<variant_index<SValueVariant, LpcArrayRef>()>(ref);
        }
    }
};
