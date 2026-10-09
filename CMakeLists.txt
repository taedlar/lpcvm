# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

cmake_minimum_required(VERSION 3.28)
project(lpcvm VERSION 1.0 LANGUAGES CXX)

include(CTest)
include(CMakeDependentOption)

cmake_dependent_option(BUILD_TESTING "Build unit tests" ON "NOT LPCVM_BUILD_TESTS" OFF)

# dependencies
include(FetchContent)

find_package(BISON 3.2 REQUIRED)
find_package(FLEX 2.5 REQUIRED)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v3.2
)
FetchContent_Declare(
	GTest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.17.0
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(argparse spdlog)

add_subdirectory(src)

# unit tests
if(BUILD_TESTING)
    FetchContent_MakeAvailable(GTest)
    add_subdirectory(tests)
endif()
