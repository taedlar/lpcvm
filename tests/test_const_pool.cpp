// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include <cmath>
#include "const_pool.hpp"

TEST(ConstTableTest, NumericRoundTrip) {
    ConstTable t;
    auto i32 = t.find_or_add(int32_t{-42});
    auto u32 = t.find_or_add(uint32_t{42});
    auto i64 = t.find_or_add(int64_t{-1234567890123LL});
    auto u64 = t.find_or_add(uint64_t{0xFFFFFFFFFFFFFFFFULL});
    auto f = t.find_or_add(1.5f);
    auto d = t.find_or_add(3.14159);

    EXPECT_EQ(t.fetch<int32_t>(i32), -42);
    EXPECT_EQ(t.fetch<uint32_t>(u32), 42u);
    EXPECT_EQ(t.fetch<int64_t>(i64), -1234567890123LL);
    EXPECT_EQ(t.fetch<uint64_t>(u64), 0xFFFFFFFFFFFFFFFFULL);
    EXPECT_FLOAT_EQ(t.fetch<float>(f), 1.5f);
    EXPECT_DOUBLE_EQ(t.fetch<double>(d), 3.14159);
}

TEST(ConstTableTest, SlotLayout) {
    ConstTable t;
    EXPECT_EQ(t.find_or_add(int32_t{1}), 0u);
    EXPECT_EQ(t.find_or_add(int64_t{2}), 1u);  // 2 slots
    EXPECT_EQ(t.find_or_add(int32_t{3}), 3u);
    EXPECT_EQ(t.slot_count(), 4u);
}

TEST(ConstTableTest, DeduplicatesNumbers) {
    ConstTable t;
    auto a = t.find_or_add(int32_t{7});
    EXPECT_EQ(t.find_or_add(int32_t{7}), a);
    EXPECT_EQ(t.slot_count(), 1u);
}

TEST(ConstTableTest, SameBitsDifferentTypeAreDistinct) {
    ConstTable t;
    auto a = t.find_or_add(int32_t{5});
    auto b = t.find_or_add(uint32_t{5});
    EXPECT_NE(a, b);
    EXPECT_EQ(t.fetch<int32_t>(a), 5);
    EXPECT_EQ(t.fetch<uint32_t>(b), 5u);
}

TEST(ConstTableTest, StringRoundTripAndDedup) {
    ConstTable t;
    auto a = t.find_or_add(std::string_view("hello"));
    auto b = t.find_or_add(std::string_view("world"));
    EXPECT_NE(a, b);
    EXPECT_EQ(t.find_or_add(std::string_view("hello")), a);
    EXPECT_EQ(t.fetch_string(a), "hello");
    EXPECT_EQ(t.fetch_string(b), "world");
}

TEST(ConstTableTest, EmptyString) {
    ConstTable t;
    auto a = t.find_or_add(std::string_view(""));
    EXPECT_EQ(t.fetch_string(a), "");
    EXPECT_EQ(t.find_or_add(std::string_view("")), a);
}

TEST(ConstTableTest, StringsSurviveGrowth) {
    ConstTable t;
    std::vector<ConstPool::IndexType> idx;
    for (int i = 0; i < 1000; ++i)
        idx.push_back(t.find_or_add(std::string_view(std::to_string(i))));
    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(t.fetch_string(idx[i]), std::to_string(i));
        EXPECT_EQ(t.find_or_add(std::string_view(std::to_string(i))), idx[i]);
    }
}

TEST(ConstTableTest, StringAndNumberCoexist) {
    ConstTable t;
    auto s = t.find_or_add(std::string_view("abc"));
    auto n = t.find_or_add(int64_t{99});
    EXPECT_EQ(t.fetch_string(s), "abc");
    EXPECT_EQ(t.fetch<int64_t>(n), 99);
}

TEST(ConstPoolTest, ReadableThroughConstBasePointer) {
    ConstTable t;
    auto n = t.find_or_add(2.5);
    auto s = t.find_or_add(std::string_view("x"));
    const ConstPool& pool = t;
    EXPECT_DOUBLE_EQ(pool.fetch<double>(n), 2.5);
    EXPECT_EQ(pool.fetch_string(s), "x");
}
