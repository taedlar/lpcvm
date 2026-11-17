// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include "fabric.hpp"
#include "vm_executor.hpp"
#include "vm_svalue.hpp"

// Public accessors expose only values, and const mappings cannot replace them.
static_assert(std::is_same_v<decltype(std::declval<LpcMapping&>().find(
    std::declval<const lpcvm::svalue_t&>())), lpcvm::svalue_t*>);
static_assert(std::is_same_v<decltype(std::declval<const LpcMapping&>().find(
    std::declval<const lpcvm::svalue_t&>())), const lpcvm::svalue_t*>);
static_assert(std::is_same_v<decltype(std::declval<LpcMapping&>().find_or_insert(
    std::declval<const lpcvm::svalue_t&>())), lpcvm::svalue_t&>);
static_assert(std::is_same_v<decltype(std::declval<LpcMapping&>()[
    std::declval<const lpcvm::svalue_t&>()]), lpcvm::svalue_t&>);

TEST(LpcMappingTest, UsesBaseFabricForInterningWithStackRuntime) {
    LpcMapping detached;
    EXPECT_EQ(detached.fabric(), nullptr);
    ManagedRuntime runtime;
    auto mapping = runtime.make_container<LpcMappingRef>(8);
    const LpcContainer& base = *mapping;
    EXPECT_EQ(base.fabric(), &runtime);
    auto array = runtime.make_container<LpcArrayRef>();
    EXPECT_EQ(array->fabric(), &runtime);

    auto buffer = runtime.make_buffer();
    buffer->data.assign("a\0b", 3);
    mapping->find_or_insert({buffer}).value = int32_t{42};
    auto interned = runtime.find_shared_string(buffer->data);
    ASSERT_TRUE(interned.has_value());
    buffer->data = "changed";
    ASSERT_NE(mapping->find({*interned}), nullptr);
    EXPECT_EQ(std::get<int32_t>(mapping->find({*interned})->value), 42);
}

TEST(LpcMappingTest, FactoryFabricDetachesWithoutLosingStoredKeys) {
    auto runtime = std::make_shared<ManagedRuntime>();
    std::weak_ptr<ManagedRuntime> weak = runtime;
    auto mapping = LpcMapping::create(runtime, 8);
    EXPECT_EQ(mapping->fabric(), runtime.get());
    EXPECT_EQ(mapping->size(), 0u);
    auto buffer = runtime->make_buffer();
    buffer->data = "stored";
    mapping->find_or_insert({buffer}).value = int32_t{42};
    auto stored = runtime->make_shared_string(buffer->data);
    runtime.reset();
    EXPECT_TRUE(weak.expired());
    EXPECT_EQ(mapping->fabric(), nullptr);
    ASSERT_NE(mapping->find({stored}), nullptr);
    EXPECT_EQ(std::get<int32_t>(mapping->find({stored})->value), 42);
    // Existing buffer keys need no interning, but new ones need a live owner.
    EXPECT_EQ(&mapping->find_or_insert({buffer}), mapping->find({stored}));
    buffer->data = "new";
    EXPECT_THROW(mapping->find_or_insert({buffer}), std::runtime_error);
    EXPECT_EQ(mapping->size(), 1u);
    mapping->find_or_insert({int32_t{7}}).value = int32_t{8};
    EXPECT_EQ(mapping->size(), 2u);
    EXPECT_THROW(LpcMapping::create(nullptr), std::invalid_argument);
}

TEST(LpcMappingTest, ValueUpdatesPreserveStoredKeyAndConstLookup) {
    LpcMapping mapping;
    const LpcMapping& view = mapping;
    lpcvm::svalue_t key{int32_t{7}};
    EXPECT_EQ(view.find(key), nullptr);
    mapping.find_or_insert(key).value = int32_t{42};
    key.value = int32_t{8};
    EXPECT_EQ(view.find(key), nullptr);
    ASSERT_NE(view.find({int32_t{7}}), nullptr);
    EXPECT_EQ(std::get<int32_t>(view.find({int32_t{7}})->value), 42);
    mapping.find({int32_t{7}})->value = int32_t{43};
    EXPECT_EQ(std::get<int32_t>(view.find({int32_t{7}})->value), 43);
    mapping[{int32_t{7}}] = lpcvm::svalue_t{int32_t{44}};
    EXPECT_EQ(std::get<int32_t>(view.find({int32_t{7}})->value), 44);
    EXPECT_EQ(mapping.size(), 1u);
    EXPECT_EQ(view.find({int32_t{44}}), nullptr);
    mapping[{int32_t{8}}].value = int32_t{45};
    EXPECT_EQ(mapping.size(), 2u);
    EXPECT_EQ(std::get<int32_t>(view.find({int32_t{8}})->value), 45);
}

TEST(LpcMappingTest, CanInsertUsingExistingValueAsKeyDuringGrowth) {
    LpcMapping mapping;
    for (int32_t i = 0; i < 8; ++i)
        mapping.find_or_insert({i}).value = int32_t{i + 100};
    auto* value = mapping.find({int32_t{0}});
    ASSERT_NE(value, nullptr);
    // This key aliases entry storage that insertion must reallocate.
    mapping.find_or_insert(*value).value = int32_t{999};
    EXPECT_EQ(mapping.size(), 9u);
    ASSERT_NE(mapping.find({int32_t{100}}), nullptr);
    EXPECT_EQ(std::get<int32_t>(mapping.find({int32_t{100}})->value), 999);
    EXPECT_EQ(std::get<int32_t>(mapping.find({int32_t{0}})->value), 100);
}

TEST(LpcMappingTest, InsertsAndReturnsExistingEntry) {
    LpcMapping mapping;
    EXPECT_EQ(mapping.find({int32_t{7}}), nullptr);
    auto* entry = &mapping.find_or_insert({int32_t{7}});
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(mapping.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(entry->value));
    entry->value = int32_t{42};
    EXPECT_EQ(&mapping.find_or_insert({int32_t{7}}), entry);
    EXPECT_EQ(&mapping[{int32_t{7}}], entry);
    EXPECT_EQ(mapping.size(), 1u);
    EXPECT_EQ(std::get<int32_t>(mapping.find({int32_t{7}})->value), 42);
    EXPECT_EQ(mapping.find({int32_t{8}}), nullptr);
}

TEST(LpcMappingTest, RehashesAcrossAllIndexWidths) {
    LpcMapping mapping;
    constexpr int32_t count = 33000; // crosses both 128- and 32768-entry boundaries
    for (int32_t i = 0; i < count; ++i) {
        auto* entry = &mapping.find_or_insert({i});
        ASSERT_NE(entry, nullptr);
        entry->value = int32_t{i + 1};
        if (i == 128 || i == 32768) {
            for (int32_t j = 0; j <= i; ++j) {
                auto* found = mapping.find({j});
                ASSERT_NE(found, nullptr) << j;
                EXPECT_EQ(std::get<int32_t>(found->value), j + 1);
            }
        }
    }
    EXPECT_EQ(mapping.size(), static_cast<std::size_t>(count));
    for (int32_t i = 0; i < count; ++i) {
        auto* entry = mapping.find({i});
        ASSERT_NE(entry, nullptr) << i;
        EXPECT_EQ(std::get<int32_t>(entry->value), i + 1);
        EXPECT_EQ(&mapping.find_or_insert({i}), entry);
    }
    EXPECT_EQ(mapping.size(), static_cast<std::size_t>(count));
    EXPECT_EQ(mapping.find({count}), nullptr);
}

TEST(LpcMappingTest, ProbesCollisionsAndMissingKeys) {
    LpcMapping mapping;
    std::vector<int32_t> keys;
    for (int32_t i = 0; keys.size() < 5; ++i) {
        if ((LpcMapping::hash({i}) & 7) == 0)
            keys.push_back(i);
    }
    for (std::size_t i = 0; i < 4; ++i)
        mapping.find_or_insert({keys[i]}).value = int32_t{42};
    auto* existing = mapping.find({keys[0]});
    EXPECT_EQ(&mapping.find_or_insert({keys[0]}), existing); // no growth on a hit
    EXPECT_EQ(mapping.find({keys[4]}), nullptr);
    mapping.find_or_insert({keys[4]}); // grow while preserving collided entries
    for (auto key : keys)
        EXPECT_NE(mapping.find({key}), nullptr);
    EXPECT_EQ(mapping.size(), 5u);
}

TEST(LpcMappingTest, ReservedSlotsAreNotEntries) {
    for (std::size_t reserve : {1u, 8u, 9u, 32u}) {
        LpcMapping mapping(reserve);
        EXPECT_EQ(mapping.size(), 0u);
        EXPECT_EQ(mapping.find({SValue{}}), nullptr);
        for (int32_t i = 0; i < 40; ++i)
            mapping.find_or_insert({i});
        EXPECT_EQ(mapping.size(), 40u);
        for (int32_t i = 0; i < 40; ++i)
            EXPECT_NE(mapping.find({i}), nullptr);
    }
    EXPECT_THROW((LpcMapping(std::numeric_limits<std::size_t>::max())), std::length_error);
}

TEST(LpcMappingTest, InternsBufferKeysAndMatchesStringRepresentations) {
    auto runtime = std::make_shared<ManagedRuntime>();
    auto mapping_ref = LpcMapping::create(runtime);
    auto& mapping = *mapping_ref;
    auto buffer = runtime->make_buffer();
    buffer->data.assign("a\0b", 3);
    auto shared = runtime->make_shared_string(buffer->data);
    auto* entry = &mapping.find_or_insert({buffer});
    entry->value = int32_t{123};
    EXPECT_EQ(&mapping.find_or_insert({shared}), entry);
    EXPECT_EQ(mapping.size(), 1u);
    buffer->data = "changed";
    EXPECT_EQ(mapping.find({buffer}), nullptr);
    for (int32_t i = 0; i < 150; ++i)
        mapping.find_or_insert({i});
    ASSERT_NE(mapping.find({shared}), nullptr);
    EXPECT_EQ(std::get<int32_t>(mapping.find({shared})->value), 123);
}

TEST(LpcMappingTest, RejectsUnresolvableKeysWithoutInserting) {
    LpcMapping mapping;
    lpcvm::DataStack stack;
    stack.push_temp_string("temporary");
    EXPECT_THROW(mapping.find_or_insert({stack.top()}), std::invalid_argument);
    auto runtime = std::make_shared<ManagedRuntime>();
    auto buffer = runtime->make_buffer();
    EXPECT_THROW(mapping.find_or_insert({buffer}), std::runtime_error);
    auto expired = runtime->make_container<LpcMappingRef>();
    runtime.reset();
    EXPECT_THROW(expired->find_or_insert({buffer}), std::runtime_error);
    EXPECT_EQ(mapping.size(), 0u);
    EXPECT_EQ(expired->size(), 0u);
    mapping.find_or_insert({int32_t{1}});
    EXPECT_NE(mapping.find({int32_t{1}}), nullptr);
}

TEST(LpcMappingTest, ReleasesEntriesAfterGrowth) {
    auto array = lpcvm::create_shared<LpcArray>();
    lpcvm::Handle<LpcArray, LpcArrayRef> weak(array);
    {
        LpcMapping mapping;
        for (int32_t i = 0; i < 100; ++i)
            mapping.find_or_insert({i}).value = array;
        EXPECT_EQ(array.use_count(), 101u);
        array.reset();
        EXPECT_FALSE(weak.expired());
    }
    EXPECT_TRUE(weak.expired());
}

TEST(LpcMappingTest, HonorsValueEqualityForSpecialKeys) {
    LpcMapping mapping;
    mapping.find_or_insert({0.0});
    mapping.find_or_insert({-0.0});
    EXPECT_EQ(mapping.size(), 1u);
    mapping.find_or_insert({int32_t{0}});
    EXPECT_EQ(mapping.size(), 2u);
    mapping.find_or_insert({LpcStringRef{}});
    mapping.find_or_insert({LpcBufferRef{}});
    EXPECT_EQ(mapping.size(), 3u);
    mapping.find_or_insert({SValue{}});
    EXPECT_EQ(mapping.size(), 4u);
    EXPECT_NE(mapping.find({0.0}), nullptr);
    EXPECT_NE(mapping.find({int32_t{0}}), nullptr);
    EXPECT_NE(mapping.find({LpcStringRef{}}), nullptr);
    EXPECT_NE(mapping.find({SValue{}}), nullptr);
}

TEST(LpcMappingTest, ErasePreservesCollisionChainsAndAvoidsDuplicateKeys) {
    LpcMapping mapping;
    std::vector<int32_t> keys;
    for (int32_t i = 0; keys.size() < 4; ++i) {
        if ((LpcMapping::hash({i}) & 7) == 0)
            keys.push_back(i);
    }
    mapping.erase({keys[0]}); // empty mapping
    for (int32_t key : keys)
        mapping[{key}].value = key;
    mapping.erase({keys[0]}); // first bucket in the collision chain
    EXPECT_EQ(mapping.size(), 3u);
    EXPECT_EQ(mapping.find({keys[0]}), nullptr);
    mapping.erase({keys[0]}); // already absent
    for (std::size_t i = 1; i < keys.size(); ++i) {
        ASSERT_NE(mapping.find({keys[i]}), nullptr);
        EXPECT_EQ(std::get<int32_t>(mapping.find_or_insert({keys[i]}).value), keys[i]);
    }
    EXPECT_EQ(mapping.size(), 3u);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(mapping.find_or_insert({keys[0]}).value));
    EXPECT_EQ(mapping.size(), 4u);
    for (auto key : keys)
        mapping.erase({key});
    EXPECT_EQ(mapping.size(), 0u);
    mapping[{keys[0]}].value = int32_t{42};
    EXPECT_EQ(mapping.size(), 1u);
    EXPECT_EQ(std::get<int32_t>(mapping.find({keys[0]})->value), 42);
}

TEST(LpcMappingTest, EraseCompactsEntriesAcrossAllIndexWidths) {
    for (int32_t count : {4, 200, 33000}) {
        SCOPED_TRACE(count);
        LpcMapping mapping;
        for (int32_t i = 0; i < count; ++i)
            mapping[{i}].value = int32_t{i + 100};
        for (int32_t i = 0; i < count; i += 2)
            mapping.erase({i});
        EXPECT_EQ(mapping.size(), static_cast<std::size_t>(count / 2));
        for (int32_t i = 0; i < count; ++i) {
            const auto* value = mapping.find({i});
            if (i % 2 == 0) {
                EXPECT_EQ(value, nullptr);
            } else {
                ASSERT_NE(value, nullptr) << i;
                EXPECT_EQ(std::get<int32_t>(value->value), i + 100);
            }
        }
        for (int32_t i = 0; i < count; i += 2)
            mapping[{i}].value = int32_t{i + 100};
        for (int32_t i = 0; i < count; ++i) {
            ASSERT_NE(mapping.find({i}), nullptr) << i;
            EXPECT_EQ(std::get<int32_t>(mapping.find({i})->value), i + 100);
        }
    }
}

TEST(LpcMappingTest, RepeatedEraseInsertDoesNotExhaustEmptyBuckets) {
    LpcMapping mapping;
    mapping[{int32_t{0}}].value = int32_t{42};
    for (int32_t i = 1; i < 2000; ++i) {
        mapping[{i}].value = i;
        mapping.erase({i});
        EXPECT_EQ(mapping.find({i}), nullptr);
        EXPECT_EQ(mapping.size(), 1u);
        ASSERT_NE(mapping.find({int32_t{0}}), nullptr);
        EXPECT_EQ(std::get<int32_t>(mapping.find({int32_t{0}})->value), 42);
    }
}

TEST(LpcMappingTest, EraseReleasesKeyAndValueReferences) {
    LpcMapping mapping;
    auto key = lpcvm::create_shared<LpcArray>();
    auto value = lpcvm::create_shared<LpcArray>();
    lpcvm::Handle<LpcArray, LpcArrayRef> weak(value);
    mapping[{key}].value = value;
    mapping[{int32_t{1}}].value = int32_t{42}; // last entry must move
    value.reset();
    EXPECT_EQ(key.use_count(), 2u);
    mapping.erase({key});
    EXPECT_EQ(key.use_count(), 1u);
    EXPECT_TRUE(weak.expired());
    ASSERT_NE(mapping.find({int32_t{1}}), nullptr);
    EXPECT_EQ(std::get<int32_t>(mapping.find({int32_t{1}})->value), 42);
}

TEST(LpcMappingTest, EraseAcceptsAliasedValuesAndStringRepresentations) {
    LpcMapping mapping;
    mapping[{int32_t{1}}].value = int32_t{1};
    mapping[{int32_t{2}}].value = int32_t{2};
    mapping.erase(*mapping.find({int32_t{1}}));
    EXPECT_EQ(mapping.find({int32_t{1}}), nullptr);
    mapping.erase(*mapping.find({int32_t{2}}));
    EXPECT_EQ(mapping.size(), 0u);

    auto runtime = std::make_shared<ManagedRuntime>();
    auto buffer = runtime->make_buffer();
    buffer->data.assign("a\0b", 3);
    mapping[{runtime->make_shared_string(buffer->data)}].value = int32_t{42};
    mapping.erase({buffer});
    EXPECT_EQ(mapping.size(), 0u);
    mapping[{0.0}];
    mapping.erase({-0.0});
    EXPECT_EQ(mapping.size(), 0u);
    mapping[{LpcStringRef{}}];
    mapping.erase({LpcBufferRef{}});
    EXPECT_EQ(mapping.size(), 0u);
    lpcvm::DataStack stack;
    stack.push_temp_string("temporary");
    EXPECT_THROW(mapping.erase({stack.top()}), std::invalid_argument);
}
