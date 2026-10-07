# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

cmake_minimum_required(VERSION 3.28)
project(lpcvm VERSION 1.0 LANGUAGES CXX)

# dependencies
include(FetchContent)

find_package(BISON 3.2 REQUIRED)
find_package(FLEX 2.5 REQUIRED)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_Declare(
	GTest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.17.0
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(argparse)

add_subdirectory(src)

# unit tests
include(CTest)
if(BUILD_TESTING)
    FetchContent_MakeAvailable(GTest)
    add_subdirectory(tests)
endif()
