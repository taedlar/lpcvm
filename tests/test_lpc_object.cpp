// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>
#include "lpc_types.hpp"

TEST(LpcObjectHandleTest, LockRetainsObjectUntilStrongReferenceIsReleased) {
    LpcHandle<LpcObject> handle{LpcHandle<LpcObject>::invalid_index, 0};
    std::optional<LpcObjectRef> locked;
    {
        LpcObjectRef owner{std::make_shared<RuntimeClass>()};
        handle = owner->handle();
        locked = handle.lock();
        ASSERT_TRUE(locked.has_value());
        EXPECT_EQ((*locked)->handle().index, handle.index);
        EXPECT_EQ((*locked)->handle().generation, handle.generation);
    }

    EXPECT_TRUE(handle.lock().has_value());
    locked.reset();
    EXPECT_FALSE(handle.lock().has_value());
}

TEST(LpcObjectHandleTest, ReusesSlotsAndRejectsStaleRelease) {
    auto* object = reinterpret_cast<LpcObject*>(std::uintptr_t{1});
    const auto stale_handle = LpcHandle<LpcObject>::create_handle(object);
    LpcHandle<LpcObject>::release_handle(stale_handle);

    const auto current_handle = LpcHandle<LpcObject>::create_handle(object);
    EXPECT_EQ(current_handle.index, stale_handle.index);
    EXPECT_NE(current_handle.generation, stale_handle.generation);

    LpcHandle<LpcObject>::release_handle(stale_handle);
    const auto other_handle = LpcHandle<LpcObject>::create_handle(object);
    EXPECT_NE(other_handle.index, current_handle.index);

    LpcHandle<LpcObject>::release_handle(current_handle);
    LpcHandle<LpcObject>::release_handle(other_handle);
}

TEST(LpcObjectHandleTest, AllocatesAndRecyclesSlotsOnOwningThread) {
    constexpr std::size_t handle_count = 9000;
    auto* object = reinterpret_cast<LpcObject*>(std::uintptr_t{1});
    std::vector<LpcHandle<LpcObject>> handles;
    handles.reserve(handle_count);
    for (std::size_t i = 0; i < handle_count; ++i) {
        handles.push_back(LpcHandle<LpcObject>::create_handle(object));
    }

    std::unordered_set<uint32_t> indices;
    for (const auto handle : handles) {
        EXPECT_TRUE(indices.insert(handle.index).second);
    }

    for (const auto handle : handles) {
        LpcHandle<LpcObject>::release_handle(handle);
    }

    const auto recycled = LpcHandle<LpcObject>::create_handle(object);
    EXPECT_EQ(recycled.index, handles.back().index);
    EXPECT_NE(recycled.generation, handles.back().generation);
    LpcHandle<LpcObject>::release_handle(recycled);
}