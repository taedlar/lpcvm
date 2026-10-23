// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_types.hpp"
#include "runtime_class.hpp"
#include "runtime_state.hpp"

std::unordered_map<std::string, LpcObjectRef> LpcObject::named_collection{};

LpcObject::LpcObject(std::shared_ptr<RuntimeClass> program)
    : program_(std::move(program)), state_(std::make_shared<RuntimeState>(program_->num_instance_variables())) {

}