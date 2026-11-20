// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "fabric.hpp"
#include "lpc_types.hpp"
#include "vm_svalue.hpp"

#include <gtest/gtest.h>

using ArrayHandle = lpcvm::Handle<LpcArray, LpcArrayRef>;

TEST(LpcGarbageCollectionTest, CollectsSelfCycleAndReleasesPayload) {
    ManagedRuntime runtime;
    auto array = runtime.make_container<LpcArrayRef>(2);
    ArrayHandle weak(array);
    (*array)[0].value = array;
    (*array)[1].value = runtime.make_shared_string("payload");
    EXPECT_EQ(runtime.collect_garbage(), 0);
    EXPECT_EQ(array.use_count(), 2);
    array.reset();
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(runtime.collect_garbage(), 1);
    EXPECT_TRUE(weak.expired());
    EXPECT_FALSE(weak.lock().has_value());
    EXPECT_GT(runtime.sweep_shared_strings(), 0);
    EXPECT_EQ(runtime.collect_garbage(), 0);
}

TEST(LpcGarbageCollectionTest, CollectsCycleWithDuplicateEdgesAndDescendants) {
    ManagedRuntime runtime;
    auto first = runtime.make_container<LpcArrayRef>(2);
    auto second = runtime.make_container<LpcArrayRef>(2);
    auto leaf = runtime.make_container<LpcMappingRef>();
    ArrayHandle weak_first(first), weak_second(second);
    lpcvm::Handle<LpcMapping, LpcMappingRef> weak_leaf(leaf);
    (*first)[0].value = second;
    (*first)[1].value = second;
    (*second)[0].value = first;
    (*second)[1].value = leaf;
    first.reset();
    second.reset();
    leaf.reset();
    EXPECT_EQ(runtime.collect_garbage(), 3);
    EXPECT_TRUE(weak_first.expired());
    EXPECT_TRUE(weak_second.expired());
    EXPECT_TRUE(weak_leaf.expired());
}

TEST(LpcGarbageCollectionTest, MarksTransitivelyFromExternalRoots) {
    ManagedRuntime runtime;
    auto root = runtime.make_container<LpcArrayRef>(1);
    auto first = runtime.make_container<LpcArrayRef>(1);
    auto second = runtime.make_container<LpcArrayRef>(1);
    ArrayHandle weak_first(first), weak_second(second);
    (*root)[0].value = first;
    (*first)[0].value = second;
    (*second)[0].value = first;
    first.reset();
    second.reset();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    EXPECT_FALSE(weak_first.expired());
    EXPECT_FALSE(weak_second.expired());
    root->clear();
    EXPECT_EQ(runtime.collect_garbage(), 2);
    EXPECT_EQ(root->size(), 0);
}

TEST(LpcGarbageCollectionTest, GarbageCanReferenceLiveContainer) {
    ManagedRuntime runtime;
    auto live = runtime.make_container<LpcArrayRef>(1);
    (*live)[0].value = int32_t{42};
    auto garbage = runtime.make_container<LpcArrayRef>(2);
    (*garbage)[0].value = garbage;
    (*garbage)[1].value = live;
    garbage.reset();
    EXPECT_EQ(runtime.collect_garbage(), 1);
    EXPECT_EQ(live.use_count(), 1);
    EXPECT_EQ(std::get<int32_t>((*live)[0].value), 42);
}

TEST(LpcGarbageCollectionTest, UntrackedAndForeignReferencesAreRoots) {
    ManagedRuntime runtime, other;
    auto array = runtime.make_container<LpcArrayRef>(1);
    auto foreign = other.make_container<LpcArrayRef>(1);
    LpcArray untracked(1);
    ArrayHandle weak(array);
    (*array)[0].value = array;
    (*foreign)[0].value = array;
    untracked[0].value = array;
    array.reset();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    EXPECT_EQ(other.collect_garbage(), 0);
    foreign->clear();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    untracked.clear();
    EXPECT_EQ(runtime.collect_garbage(), 1);
    EXPECT_TRUE(weak.expired());
}

TEST(LpcGarbageCollectionTest, TraversesInitializedSlotsAfterResizeAndClear) {
    ManagedRuntime runtime;
    auto array = runtime.make_container<LpcArrayRef>(1);
    array->resize(8);
    array->resize(2);
    array->resize(5);
    for (std::size_t i = 0; i < array->size(); ++i)
        EXPECT_TRUE(std::holds_alternative<std::monostate>((*array)[i].value));
    EXPECT_EQ(runtime.collect_garbage(), 0);
    array->clear();
    array->resize(3);
    (*array)[2].value = array;
    array.reset();
    EXPECT_EQ(runtime.collect_garbage(), 1);
}

TEST(LpcGarbageCollectionTest, LargeCycleUsesIterativeTraversalAndTeardown) {
    ManagedRuntime runtime;
    constexpr std::size_t count = 10000;
    auto first = runtime.make_container<LpcArrayRef>(1);
    auto last = first;
    for (std::size_t i = 1; i < count; ++i) {
        auto next = runtime.make_container<LpcArrayRef>(1);
        (*last)[0].value = next;
        last = next;
    }
    (*last)[0].value = first;
    last.reset();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    first.reset();
    EXPECT_EQ(runtime.collect_garbage(), count);
    EXPECT_EQ(runtime.collect_garbage(), 0);
}

TEST(LpcGarbageCollectionTest, CycleAcrossRuntimesIsPreserved) {
    ManagedRuntime runtime, other;
    auto first = runtime.make_container<LpcArrayRef>(1);
    auto second = other.make_container<LpcArrayRef>(1);
    ArrayHandle weak(first);
    (*first)[0].value = second;
    (*second)[0].value = first;
    first.reset();
    second.reset();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    EXPECT_EQ(other.collect_garbage(), 0);
    auto retained = weak.lock();
    ASSERT_TRUE(retained);
    (*retained)->clear(); // Break the cross-runtime cycle explicitly.
    retained.reset();
    EXPECT_TRUE(weak.expired());
}

TEST(LpcArrayTest, FactoryCreatesSharedContainers) {
    auto runtime = std::make_shared<ManagedRuntime>();
    auto array = LpcArray::create(runtime, 3);
    EXPECT_EQ(array->size(), 3);
    EXPECT_EQ(array.use_count(), 1);
    auto copy = array;
    EXPECT_EQ(copy.use_count(), 2);
    EXPECT_EQ(LpcArray::create(runtime)->size(), 0);
    EXPECT_TRUE(runtime->make_container<LpcMappingRef>());
}

TEST(LpcArrayTest, ContainerListSupportsRemovalAndReuse) {
    ManagedRuntime runtime;
    auto tail = runtime.make_container<LpcArrayRef>();
    auto middle = runtime.make_container<LpcArrayRef>();
    auto head = runtime.make_container<LpcArrayRef>();
    auto new_head = runtime.make_container<LpcArrayRef>();
    new_head.reset(); // Removing the head must retain the rest of the list.
    EXPECT_EQ(runtime.collect_garbage(), 0);
    middle.reset();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    tail.reset();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    head.reset();
    EXPECT_EQ(runtime.collect_garbage(), 0);
    auto replacement = runtime.make_container<LpcArrayRef>(2);
    EXPECT_EQ(replacement->size(), 2);
}

TEST(LpcArrayTest, ContainersOutliveRuntime) {
    LpcArrayRef first;
    LpcArrayRef second;
    {
        ManagedRuntime runtime;
        first = runtime.make_container<LpcArrayRef>(1);
        second = runtime.make_container<LpcArrayRef>(2);
    }
    EXPECT_EQ(first->size(), 1);
    EXPECT_EQ(second->size(), 2);
    first.reset();
    second.reset();
}

TEST(LpcArrayTest, ArrayCreateDestroy) {
    // Test creating and destroying an LpcArray
    LpcArray array(5);
    EXPECT_EQ(array.size(), 5);

    array.resize(10);
    EXPECT_EQ(array.size(), 10);

    ManagedRuntime runtime;
    array[0].value = runtime.make_shared_string("test");
    auto test_str = runtime.make_shared_string("test");
    EXPECT_EQ(test_str.use_count(), 3); // used by fabric, array, test_str
    EXPECT_EQ(std::holds_alternative<LpcStringRef>(array[0].value), true);
    EXPECT_EQ(std::get<LpcStringRef>(array[0].value), test_str);

    // Reset the test string and clear the array to release references
    test_str.reset();
    array.clear();
    EXPECT_EQ(array.size(), 0);
    runtime.sweep_shared_strings();
    EXPECT_EQ(runtime.find_shared_string("test").has_value(), false);
}
