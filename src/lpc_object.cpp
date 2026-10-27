// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_types.hpp"
#include "object_class.hpp"
#include "object_state.hpp"

std::unordered_map<std::string_view, LpcObjectRef> LpcObject::named_collection{};

LpcObject::LpcObject(std::shared_ptr<ObjectClass> program)
    : program_(std::move(program)), state_(std::make_shared<ObjectState>(program_->num_variables_inheriting())) {

}