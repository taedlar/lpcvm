# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

add_executable(lpcshell
    main.cpp
)

target_link_libraries(lpcshell PRIVATE spdlog argparse lpcvm)
