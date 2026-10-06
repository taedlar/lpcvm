# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

cmake_minimum_required(VERSION 3.28)
project(lpcvm VERSION 1.0 LANGUAGES CXX)

find_package(BISON 3.2 REQUIRED)
find_package(FLEX 2.5 REQUIRED)

add_subdirectory(src)
