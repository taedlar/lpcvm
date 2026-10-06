# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

bison_target(LPC_PARSER
    lpc_parser.yy
    ${CMAKE_CURRENT_BINARY_DIR}/lpc_parser.cpp
    DEFINES_FILE ${CMAKE_BINARY_DIR}/lpc_parser.hpp
)

flex_target(LPC_LEXER
    lpc_lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lpc_lexer.cpp
    DEFINES_FILE ${CMAKE_BINARY_DIR}/lpc_lexer.hpp
)

add_library(lpcvm ${BISON_LPC_PARSER_OUTPUTS} ${FLEX_LPC_LEXER_OUTPUTS})

target_include_directories(lpcvm PUBLIC ${CMAKE_BINARY_DIR})
