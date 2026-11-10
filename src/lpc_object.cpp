// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_types.hpp"
#include "object_class.hpp"
#include "object_state.hpp"
#include "vm_sandbox.hpp"

LpcObject::LpcObject(std::shared_ptr<FabricType> sb, std::shared_ptr<ObjectClass> program)
    : weak_fabric_(sb),
      program_(std::move(program)), state_(nullptr) {
    if (program_) {
        state_ = std::make_unique<ObjectState>(program_->num_variables_inheriting());
    }
}

LpcObject::~LpcObject() = default;

LpcObject::HandleType LpcObject::create(std::shared_ptr<FabricType> sb, std::string_view name, std::shared_ptr<ObjectClass> program) {
    if (name.empty())
        throw std::invalid_argument("Object name cannot be empty");
    if (!sb)
        throw std::invalid_argument("Named object requires a valid fabric");
    auto obj_ref = lpcvm::create_shared<LpcObject>(sb, program);
    obj_ref->name_ = sb->make_shared_string(name);
    return sb->emplace_object(obj_ref);
}
