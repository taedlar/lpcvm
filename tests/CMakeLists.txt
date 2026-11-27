# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

include(GoogleTest)

# Tests for the lpcvm library
add_executable(test_lpcvm
    test_const_pool.cpp
    test_lpc_compiler.cpp
    test_lpc_lexer.cpp
    test_native_call.cpp
    test_lpc_object.cpp
    test_lpc_string.cpp
    test_lpc_buffer.cpp
    test_vm_executor.cpp
    test_vm_svalue.cpp
    test_lpc_parser.cpp
    test_vm_sandbox.cpp
    test_preprocessor.cpp
    test_symbol_table.cpp
    test_vm_exception.cpp
    test_vm_data_stack.cpp
    test_vm_string_operator.cpp
    test_lpc_array.cpp
    test_lpc_mapping.cpp
)
target_link_libraries(test_lpcvm PRIVATE lpcvm spdlog GTest::gtest_main)
gtest_discover_tests(test_lpcvm)

# Tests for the lpcfront tool
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

# Tests for the lpcshell modules
get_target_property(LPCSHELL_SRC lpcshell SOURCE_DIR)
add_executable(test_lpcshell
    test_console_module.cpp
    ${LPCSHELL_SRC}/console_module.cpp
)
target_link_libraries(test_lpcshell PRIVATE lpcvm spdlog GTest::gtest_main)
gtest_discover_tests(test_lpcshell)
