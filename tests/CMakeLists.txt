# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

add_executable(test_lpcvm
    test_lpc_compiler.cpp
    test_lpc_lexer.cpp
)

target_link_libraries(test_lpcvm PRIVATE lpcvm GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_lpcvm)
