// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_types.hpp"
#include "object_class.hpp"
#include "object_state.hpp"
#include "vm_sandbox.hpp"

LpcObject::LpcObject(FabricType *sb, std::shared_ptr<ObjectClass> program)
    : fabric_(sb), program_(std::move(program)), state_(nullptr) {
    if (program_) {
        state_ = std::make_shared<ObjectState>(program_->num_variables_inheriting());
    }
}

LpcObject::HandleType LpcObject::create(FabricType *sb, std::string_view name, std::shared_ptr<ObjectClass> program) {
    auto obj_ref = lpcvm::create_shared<LpcObject>(sb, program);
    obj_ref->name_ = std::string(name);
    return sb->emplace_object(obj_ref->name_, obj_ref);
}
