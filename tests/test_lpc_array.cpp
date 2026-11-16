// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "fabric.hpp"
#include "lpc_types.hpp"
#include "vm_svalue.hpp"

#include <gtest/gtest.h>

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
