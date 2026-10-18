# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

add_executable(test_lpcvm
    test_const_pool.cpp
    test_lpc_compiler.cpp
    test_lpc_lexer.cpp
    test_lpc_object.cpp
    test_vm_executor.cpp
    test_vm_svalue.cpp
)

target_link_libraries(test_lpcvm PRIVATE lpcvm GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_lpcvm)

add_test(NAME LpcFrontTest.Example01
    COMMAND lpcfront ${PROJECT_SOURCE_DIR}/src/lpcfront/example01.lpc)
set_tests_properties(LpcFrontTest.Example01 PROPERTIES
    FAIL_REGULAR_EXPRESSION "parsing error;failed:")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/invalid.lpc "int string x;\n")
add_test(NAME LpcFrontTest.InvalidSource
    COMMAND lpcfront ${CMAKE_CURRENT_BINARY_DIR}/invalid.lpc)
add_test(NAME LpcFrontTest.MissingSource
    COMMAND lpcfront ${CMAKE_CURRENT_BINARY_DIR}/missing.lpc)
set_tests_properties(LpcFrontTest.InvalidSource LpcFrontTest.MissingSource
    PROPERTIES WILL_FAIL TRUE)
