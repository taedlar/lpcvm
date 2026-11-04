// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include "vm_svalue.hpp"

static_assert(std::is_constructible_v<SValue, LpcStringRef>);
static_assert(sizeof(void*) != 8 || sizeof(SValue) == 16,
              "SValue must remain 16 bytes on 64-bit architectures");
