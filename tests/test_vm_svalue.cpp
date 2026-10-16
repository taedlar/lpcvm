// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include "vm_svalue.hpp"

namespace {
using ReorderedRefs = std::variant<LpcArrayRef, LpcObjectRef, LpcMappingRef>;
static_assert(variant_index<ReorderedRefs, LpcArrayRef>() == 0);
static_assert(variant_index<ReorderedRefs, LpcObjectRef>() == 1);
static_assert(variant_index<ReorderedRefs, LpcMappingRef>() == 2);
static_assert(variant_index<ReorderedRefs, int>() == 3);

template <typename T, typename Ref, typename Registry>
void check_reference_slots(Registry& registry, std::vector<Ref>& free_refs) {
    registry.clear();
    free_refs.clear();
    registry.resize(2);
    const std::shared_ptr<T> pointer;
    const SValue fresh(pointer);
    EXPECT_TRUE(std::holds_alternative<Ref>(fresh));
    EXPECT_EQ(std::get<Ref>(fresh).index, 2u);
    EXPECT_EQ(registry.size(), 3u);

    free_refs.push_back(Ref{1});
    const SValue reused(pointer);
    EXPECT_TRUE(std::holds_alternative<Ref>(reused));
    EXPECT_EQ(std::get<Ref>(reused).index, 1u);
    EXPECT_EQ(registry.size(), 3u);
    EXPECT_TRUE(free_refs.empty());
    registry.clear();
}
}

TEST(SValueTest, ObjectReferenceSlots) {
    check_reference_slots<LpcObject>(SValue::all_object_refs, SValue::free_object_refs);
}

TEST(SValueTest, MappingReferenceSlots) {
    check_reference_slots<LpcMapping>(SValue::all_mapping_refs, SValue::free_mapping_refs);
}

TEST(SValueTest, ArrayReferenceSlots) {
    check_reference_slots<LpcArray>(SValue::all_array_refs, SValue::free_array_refs);
}
