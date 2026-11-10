// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include <gtest/gtest.h>
#include "lpc_types.hpp"
#include "fabric.hpp"
#include "vm_svalue.hpp"

static_assert(sizeof(LpcBufferRef) == sizeof(void*));
static_assert(sizeof(SValue) == 16);
static_assert(!std::is_constructible_v<LpcBuffer>);

namespace {
const std::size_t small = lpcvm::BufferPool::small_capacity();
}

TEST(LpcBufferTest, ReferencesShareMutableBytes) {
    ManagedRuntime runtime;
    auto first = runtime.make_buffer();
    auto second = first;
    second->data.append("a\0b", 3);
    EXPECT_EQ(first->data, std::string("a\0b", 3));
    EXPECT_EQ(first.use_count(), 2u);
    EXPECT_EQ(runtime.buffer_pool_stats().live_buffers, 1u);
}

TEST(LpcBufferTest, RecyclesSmallAndLargeSeparately) {
    ManagedRuntime runtime;
    const auto* small_buffer = &*runtime.make_buffer(small); // released immediately
    auto stats = runtime.buffer_pool_stats();
    EXPECT_EQ(stats.small_buffers, 1u);
    EXPECT_EQ(stats.large_buffers, 0u);

    LpcBuffer* large_buffer;
    {
        auto large = runtime.make_buffer(small + 1);
        large->data.assign(100, 'x');
        large_buffer = &*large;
    }
    stats = runtime.buffer_pool_stats();
    EXPECT_EQ(stats.small_buffers, 1u);
    EXPECT_EQ(stats.large_buffers, 1u);
    EXPECT_GE(stats.large_capacity, 100u);

    auto reused_large = runtime.make_buffer(50);
    EXPECT_EQ(&*reused_large, large_buffer);
    EXPECT_TRUE(reused_large->data.empty());
    EXPECT_GE(reused_large->data.capacity(), 100u);

    auto reused_small = runtime.make_buffer();
    EXPECT_EQ(&*reused_small, small_buffer);
    stats = runtime.buffer_pool_stats();
    EXPECT_EQ(stats.small_buffers, 0u);
    EXPECT_EQ(stats.large_buffers, 0u);
    EXPECT_EQ(stats.live_buffers, 2u);
}

TEST(LpcBufferTest, GrowsRecycledLargeBufferToRequest) {
    ManagedRuntime runtime;
    runtime.make_buffer(small + 1);
    auto buffer = runtime.make_buffer(1000);
    EXPECT_GE(buffer->data.capacity(), 1000u);
    EXPECT_EQ(runtime.buffer_pool_stats().large_capacity, 0u);
}

TEST(LpcBufferTest, HonorsLimits) {
    auto* pool = lpcvm::BufferPool::create({1, 1, 256});
    {
        auto a = pool->acquire();
        auto b = pool->acquire();
        auto big = pool->acquire(512);
        auto c = pool->acquire(100);
        auto d = pool->acquire(100);
    }
    auto stats = pool->stats();
    EXPECT_EQ(stats.small_buffers, 1u);
    EXPECT_EQ(stats.large_buffers, 1u);
    EXPECT_LE(stats.large_capacity, 256u);
    EXPECT_GT(pool->trim(), 0u);
    EXPECT_EQ(pool->stats().large_buffers, 0u);
    pool->close();
}

TEST(LpcBufferTest, BuffersOutliveRuntime) {
    LpcBufferRef survivor;
    {
        ManagedRuntime runtime;
        runtime.make_buffer(); // recycled, then freed by the runtime
        survivor = runtime.make_buffer(small * 4);
    }
    survivor->data.assign("still valid");
    EXPECT_EQ(survivor->data, "still valid");
    survivor = {}; // frees the buffer and the closed pool
}

TEST(LpcBufferTest, StoresInSValue) {
    ManagedRuntime runtime;
    SValue value = runtime.make_buffer();
    auto copy = value;
    std::get<LpcBufferRef>(copy)->data = "shared";
    EXPECT_EQ(std::get<LpcBufferRef>(value)->data, "shared");
    value = std::monostate{};
    copy = std::monostate{};
    EXPECT_EQ(runtime.buffer_pool_stats().small_buffers, 1u);
}
