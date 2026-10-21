// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "bytecode_generator.hpp"

namespace lpcvm {
    void FunctionBuilder::emit_body(const ASTNode* body) const {
        if (!body)
            return;
        // TODO: Implement function body emission
    }
} // namespace lpcvm
