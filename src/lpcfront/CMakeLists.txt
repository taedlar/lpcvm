# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

add_executable(lpcfront
    main.cpp
    vm_overheads.cpp
)

target_link_libraries(lpcfront PRIVATE spdlog argparse lpcvm)
