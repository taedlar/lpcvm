# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

cmake_minimum_required(VERSION 3.28)
project(lpcvm VERSION 1.0 LANGUAGES CXX)

find_package(BISON 3.2 REQUIRED)
find_package(FLEX 2.5 REQUIRED)

bison_target(LPC_GRAMMAR
    src/lpc_grammar.yy
    ${CMAKE_CURRENT_BINARY_DIR}/lpc_grammar.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lpc_grammar.hpp
)

flex_target(LPC_LEXER
    src/lpc_lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lpc_lexer.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lpc_lexer.hpp
)

add_library(lpcvm ${BISON_LPC_GRAMMAR_OUTPUTS} ${FLEX_LPC_LEXER_OUTPUTS})

target_include_directories(lpcvm PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
