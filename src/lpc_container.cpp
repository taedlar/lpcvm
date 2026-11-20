// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "lpc_types.hpp"

LpcContainer::~LpcContainer() {
    if (!gc_owner_)
        return;
    if (gc_prev)
        gc_prev->gc_next = gc_next;
    else
        *gc_owner_ = gc_next;
    if (gc_next)
        gc_next->gc_prev = gc_prev;
}
