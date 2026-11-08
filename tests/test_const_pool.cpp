// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cmath>
#include "const_pool.hpp"
#include "fabric.hpp"

static_assert(!std::is_default_constructible_v<ConstPool>);
static_assert(!std::is_default_constructible_v<ConstTable>);
static_assert(!std::is_constructible_v<ConstPool, ManagedRuntime*>);
static_assert(!std::is_constructible_v<ConstTable, ManagedRuntime*>);

class ConstPoolTest : public ::testing::Test {
protected:
    ManagedRuntime fabric;
};

TEST_F(ConstPoolTest, NumericRoundTrip) {
    ConstTable t(fabric);
    auto i32 = t.find_or_add(int32_t{-42});
    auto u32 = t.find_or_add(uint32_t{42});
    auto i64 = t.find_or_add(int64_t{-1234567890123LL});
    auto u64 = t.find_or_add(uint64_t{0xFFFFFFFFFFFFFFFFULL});
    auto f = t.find_or_add(1.5f);
    auto d = t.find_or_add(3.14159);
    auto pool = t.freeze();

    EXPECT_EQ(pool->fetch_numeric<int32_t>(i32), -42);
    EXPECT_EQ(pool->fetch_numeric<uint32_t>(u32), 42u);
    EXPECT_EQ(pool->fetch_numeric<int64_t>(i64), -1234567890123LL);
    EXPECT_EQ(pool->fetch_numeric<uint64_t>(u64), 0xFFFFFFFFFFFFFFFFULL);
    EXPECT_FLOAT_EQ(pool->fetch_numeric<float>(f), 1.5f);
    EXPECT_DOUBLE_EQ(pool->fetch_numeric<double>(d), 3.14159);
}

TEST_F(ConstPoolTest, SlotLayout) {
    ConstTable t(fabric);
    EXPECT_EQ(t.find_or_add(int32_t{1}), 0u);
    EXPECT_EQ(t.find_or_add(int64_t{2}), 1u);  // 2 slots
    EXPECT_EQ(t.find_or_add(int32_t{3}), 3u);
    EXPECT_EQ(t.freeze()->slot_count(), 4u);
}

TEST_F(ConstPoolTest, DeduplicatesNumbers) {
    ConstTable t(fabric);
    auto a = t.find_or_add(int32_t{7});
    EXPECT_EQ(t.find_or_add(int32_t{7}), a);
    EXPECT_EQ(t.freeze()->slot_count(), 1u);
}

TEST_F(ConstPoolTest, SameBitsDifferentTypeAreDistinct) {
    ConstTable t(fabric);
    auto a = t.find_or_add(int32_t{5});
    auto b = t.find_or_add(uint32_t{5});
    EXPECT_NE(a, b);
    auto pool = t.freeze();
    EXPECT_EQ(pool->fetch_numeric<int32_t>(a), 5);
    EXPECT_EQ(pool->fetch_numeric<uint32_t>(b), 5u);
}

TEST_F(ConstPoolTest, StringRoundTripAndDedup) {
    ConstTable t(fabric);
    auto a = t.find_or_add(std::string_view("hello"));
    auto b = t.find_or_add(std::string_view("world"));
    EXPECT_NE(a, b);
    EXPECT_EQ(t.find_or_add(std::string_view("hello")), a);
    auto pool = t.freeze();
    EXPECT_EQ(pool->fetch_string(a)->view(), "hello");
    EXPECT_EQ(pool->fetch_string(b)->view(), "world");
}

TEST_F(ConstPoolTest, EmptyString) {
    ConstTable t(fabric);
    auto a = t.find_or_add(std::string_view(""));
    EXPECT_EQ(t.find_or_add(std::string_view("")), a);
    auto pool = t.freeze();
    EXPECT_EQ(pool->fetch_string(a)->view(), "");
}

TEST_F(ConstPoolTest, StringsSurviveGrowth) {
    ConstTable t(fabric);
    std::vector<ConstPool::IndexType> idx;
    for (int i = 0; i < 1000; ++i)
        idx.push_back(t.find_or_add(std::string_view(std::to_string(i))));
    auto pool = t.freeze();
    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(pool->fetch_string(idx[i])->view(), std::to_string(i));
    }
}

TEST_F(ConstPoolTest, StringAndNumberCoexist) {
    ConstTable t(fabric);
    auto s = t.find_or_add(std::string_view("abc"));
    auto n = t.find_or_add(int64_t{99});
    auto pool = t.freeze();
    EXPECT_EQ(pool->fetch_string(s)->view(), "abc");
    EXPECT_EQ(pool->fetch_numeric<int64_t>(n), 99);
}

TEST_F(ConstPoolTest, ReadableThroughConstBasePointer) {
    ConstTable t(fabric);
    auto n = t.find_or_add(2.5);
    auto s = t.find_or_add(std::string_view("x"));
    auto pool = t.freeze();
    EXPECT_DOUBLE_EQ(pool->fetch_numeric<double>(n), 2.5);
    EXPECT_EQ(pool->fetch_string(s)->view(), "x");
}

TEST_F(ConstPoolTest, StringsAreInternedWithinTheirFabric) {
    ManagedRuntime other_fabric;
    ConstTable first(fabric);
    ConstTable second(fabric);
    ConstTable other(other_fabric);
    auto first_pool = first.freeze();
    const auto first_index = first.find_or_add(std::string_view("shared"));
    const auto second_index = second.find_or_add(std::string_view("shared"));
    const auto other_index = other.find_or_add(std::string_view("shared"));
    EXPECT_EQ(first_pool->string_count(), 0u);
    first_pool = first.freeze();
    auto first_string = first_pool->fetch_string(first_index);
    EXPECT_EQ(first_string, second.freeze()->fetch_string(second_index));
    EXPECT_EQ(first_string, fabric.make_shared_string("shared"));
    EXPECT_FALSE(first_string == other.freeze()->fetch_string(other_index));
    EXPECT_EQ(fabric.sweep_shared_strings(), 0u);
}

TEST_F(ConstPoolTest, InvalidStringIndicesThrow) {
    ConstTable table(fabric);
    auto numeric = table.find_or_add(int32_t{1});
    auto pool = table.freeze();
    EXPECT_THROW(pool->fetch_string(numeric), std::invalid_argument);
    EXPECT_THROW(pool->fetch_string(1), std::out_of_range);
    EXPECT_THROW(pool->fetch_string(ConstPool::STRING_REF_MARKER), std::out_of_range);
}

TEST(ConstPoolLifetimeTest, FrozenStringsOutliveTableAndFabric) {
    std::shared_ptr<ConstPool> pool;
    ConstPool::IndexType index;
    {
        ManagedRuntime fabric;
        ConstTable table(fabric);
        index = table.find_or_add(std::string_view("retained"));
        pool = table.freeze();
    }
    EXPECT_EQ(pool->fetch_string(index)->view(), "retained");
}
