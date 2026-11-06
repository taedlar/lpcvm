// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include <gtest/gtest.h>
#include "lpc_types.hpp"
#include "fabric.hpp"

static_assert(!std::is_copy_constructible_v<LpcString>);
static_assert(!std::is_copy_assignable_v<LpcString>);
static_assert(std::is_same_v<decltype(std::declval<LpcString&>().data()), const char*>);
static_assert(sizeof(LpcStringRef) == sizeof(void*));
static_assert(sizeof(LpcString) == sizeof(std::size_t));

class LpcStringTest : public ::testing::Test {
protected:
    ManagedRuntime runtime;
    static constexpr std::size_t overhead = sizeof(LpcStringRef::ControlBlock) + sizeof(LpcString) + 1;
};

TEST(ManagedRuntimeStringTest, SharedStringServicesDispatchVirtually) {
    class TrackingRuntime : public ManagedRuntime {
    public:
        int allocations = 0;
        mutable int queries = 0;
        int sweeps = 0;

        LpcStringRef make_shared_string(std::string_view value) override {
            ++allocations;
            return ManagedRuntime::make_shared_string(value);
        }
        std::size_t shared_string_allocation_size() const noexcept override {
            ++queries;
            return ManagedRuntime::shared_string_allocation_size();
        }
        std::size_t sweep_shared_strings() override {
            ++sweeps;
            return ManagedRuntime::sweep_shared_strings();
        }
    } runtime;
    ManagedRuntime& base = runtime;
    auto value = lpcvm::make_shared_string(base, "hello");
    EXPECT_EQ(runtime.allocations, 1);
    const auto bytes = base.shared_string_allocation_size();
    EXPECT_GT(bytes, 0u);
    EXPECT_EQ(runtime.queries, 1);
    value = {};
    EXPECT_EQ(base.sweep_shared_strings(), bytes);
    EXPECT_EQ(runtime.sweeps, 1);
}

TEST_F(LpcStringTest, ReusesLiveStringsByContent) {
    auto first = lpcvm::make_shared_string(runtime, "hello");
    std::string input = "hello";
    auto second = lpcvm::make_shared_string(runtime, input);
    EXPECT_EQ(first, second);
    EXPECT_EQ(first.use_count(), 3u);
    EXPECT_FALSE(first == lpcvm::make_shared_string(runtime, "world"));
}

TEST_F(LpcStringTest, OwnsInputAndHonorsViewLength) {
    std::string input = "hello world";
    auto value = lpcvm::make_shared_string(runtime, std::string_view(input.data(), 5));
    input.assign("changed");
    EXPECT_EQ(value->view(), "hello");
    EXPECT_EQ(value->size(), 5u);
    EXPECT_STREQ(value->c_str(), "hello");
}

TEST_F(LpcStringTest, SupportsEmptyAndEmbeddedNullStrings) {
    auto empty = lpcvm::make_shared_string(runtime, {});
    EXPECT_TRUE(empty->empty());
    EXPECT_EQ(empty->c_str()[0], '\0');
    EXPECT_EQ(empty, lpcvm::make_shared_string(runtime, ""));
    auto value = lpcvm::make_shared_string(runtime, std::string_view("a\0b", 3));
    EXPECT_EQ(value->size(), 3u);
    EXPECT_EQ(value->c_str()[3], '\0');
    EXPECT_EQ(value->view(), std::string_view("a\0b", 3));
    EXPECT_EQ(value, lpcvm::make_shared_string(runtime, std::string_view("a\0b", 3)));
    EXPECT_FALSE(value == lpcvm::make_shared_string(runtime, "a"));
}

TEST_F(LpcStringTest, LongStringsOwnTerminatedBytesAndRemainInterned) {
    std::string input(4096, 'x');
    auto value = lpcvm::make_shared_string(runtime, input);
    EXPECT_EQ(value, lpcvm::make_shared_string(runtime, input));
    input[0] = 'y';
    EXPECT_EQ(value->view(), std::string(4096, 'x'));
    EXPECT_EQ(value->c_str()[value->size()], '\0');
    EXPECT_FALSE(value == lpcvm::make_shared_string(runtime, input));
}

TEST_F(LpcStringTest, CountsDistinctAllocationsIncludingEmptyStrings) {
    EXPECT_EQ(runtime.shared_string_allocation_size(), 0u);
    EXPECT_EQ(runtime.sweep_shared_strings(), 0u);
    auto first = lpcvm::make_shared_string(runtime, "hello");
    auto duplicate = lpcvm::make_shared_string(runtime, "hello");
    EXPECT_EQ(runtime.shared_string_allocation_size(), overhead + 5);
    auto empty = lpcvm::make_shared_string(runtime, {});
    auto binary = lpcvm::make_shared_string(runtime, std::string_view("a\0b", 3));
    EXPECT_EQ(runtime.shared_string_allocation_size(), 3 * overhead + 8);
    EXPECT_EQ(runtime.sweep_shared_strings(), 0u);
}

TEST_F(LpcStringTest, RetainsUnusedStringsUntilSweepAndAllowsRecreation) {
    lpcvm::Handle<LpcString, LpcStringRef> weak;
    {
        auto value = lpcvm::make_shared_string(runtime, "temporary");
        weak = decltype(weak)(value);
        EXPECT_EQ(value.use_count(), 2u); // Caller and runtime.
    }
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(runtime.shared_string_allocation_size(), overhead + 9);
    EXPECT_EQ(runtime.sweep_shared_strings(), overhead + 9);
    EXPECT_TRUE(weak.expired());
    EXPECT_FALSE(weak.lock().has_value());
    EXPECT_EQ(runtime.shared_string_allocation_size(), 0u);
    EXPECT_EQ(runtime.sweep_shared_strings(), 0u);
    auto recreated = lpcvm::make_shared_string(runtime, "temporary");
    EXPECT_EQ(recreated->view(), "temporary");
    EXPECT_EQ(recreated, lpcvm::make_shared_string(runtime, "temporary"));
    EXPECT_EQ(runtime.shared_string_allocation_size(), overhead + 9);
}

TEST_F(LpcStringTest, SweepPreservesExternallyOwnedStrings) {
    auto retained = lpcvm::make_shared_string(runtime, "retained");
    auto copy = retained;
    retained = {};
    lpcvm::make_shared_string(runtime, "discarded");
    lpcvm::make_shared_string(runtime, "");
    EXPECT_EQ(runtime.sweep_shared_strings(), 2 * overhead + 9);
    EXPECT_EQ(copy->view(), "retained");
    EXPECT_EQ(runtime.shared_string_allocation_size(), overhead + 8);
    EXPECT_EQ(copy, lpcvm::make_shared_string(runtime, "retained"));
    copy = {};
    EXPECT_EQ(runtime.sweep_shared_strings(), overhead + 8);
    EXPECT_EQ(runtime.shared_string_allocation_size(), 0u);
}

TEST_F(LpcStringTest, InternsPerRuntimeAndOutlivesRuntime) {
    auto first = lpcvm::make_shared_string(runtime, "shared");
    LpcStringRef retained;
    {
        ManagedRuntime other;
        retained = lpcvm::make_shared_string(other, "shared");
        EXPECT_FALSE(first == retained);
        EXPECT_EQ(retained, lpcvm::make_shared_string(other, "shared"));
        EXPECT_EQ(other.shared_string_allocation_size(), overhead + 6);
        EXPECT_EQ(other.sweep_shared_strings(), 0u);
    }
    EXPECT_EQ(retained.use_count(), 1u);
    EXPECT_EQ(retained->view(), "shared");
    EXPECT_EQ(runtime.shared_string_allocation_size(), overhead + 6);
}

