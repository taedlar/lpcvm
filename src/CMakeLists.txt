# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

bison_target(LPC_PARSER
    lpc_parser.yy
    ${CMAKE_CURRENT_BINARY_DIR}/lpc_parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lpc_parser.hpp
)

flex_target(LPC_LEXER
    lpc_lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lpc_lexer.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lpc_lexer.hpp
)

add_library(lpcvm_obj OBJECT
    ${BISON_LPC_PARSER_OUTPUTS}
    ${FLEX_LPC_LEXER_OUTPUTS}
    lpc_compiler.cpp
    lpc_compiler.hpp
    vm_executor.hpp
    vm_executor.cpp
    vm_opcodes.hpp
    source_exception.hpp
    ast_node.hpp
    symbol_table.hpp
    symbol_table.cpp
    const_pool.hpp
    const_pool.cpp
    object_class.hpp
    object_class.cpp
    virtual_table.hpp
    virtual_table.cpp
    vm_svalue.hpp
    vm_svalue.cpp
    lpc_types.hpp
    lpc_object.cpp
    code_generator.hpp
    code_generator.cpp
    object_state.hpp
    vm_sandbox.hpp
    vm_sandbox.cpp
)

target_include_directories(lpcvm_obj PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(lpcvm_obj PUBLIC spdlog)

add_library(lpcvm STATIC $<TARGET_OBJECTS:lpcvm_obj>)

target_include_directories(lpcvm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(lpcfront)
