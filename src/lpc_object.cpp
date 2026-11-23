// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_types.hpp"
#include "object_class.hpp"
#include "object_state.hpp"
#include "vm_executor.hpp"
#include "vm_sandbox.hpp"

LpcObject::LpcObject(std::shared_ptr<FabricType> sb, std::shared_ptr<ObjectClass> program)
    : lpcvm::Object(sb),
      program_(std::move(program)), state_(nullptr) {
    if (program_) {
        state_ = std::make_unique<ObjectState>(program_->num_variables_inheriting());
    }
}

LpcObject::~LpcObject() = default;

int LpcObject::call_method(void* func_identity, lpcvm::DataStack* data_stack,
                           lpcvm::ReturnStack* return_stack, int num_args) {
    if (!func_identity || !data_stack || !return_stack || !program_ ||
        num_args < 0 || static_cast<std::size_t>(num_args) > data_stack->size())
        return -1;

    lpcvm::Execution entry(retain_fabric());
    if (!program_->lookup_method(static_cast<LpcString*>(func_identity)->view(), &entry))
        return -1;

    entry.self_ = state_.get();
    entry.obj_ref_ = LpcObjectRef::retain(this);
    return entry.execute(*return_stack, *data_stack, static_cast<std::size_t>(num_args));
}

LpcObject::HandleType LpcObject::create(std::shared_ptr<FabricType> sb, std::string_view name, std::shared_ptr<ObjectClass> program) {
    if (name.empty())
        throw std::invalid_argument("Object name cannot be empty");
    if (!sb)
        throw std::invalid_argument("Named object requires a valid fabric");
    auto obj_ref = lpcvm::create_shared<LpcObject>(sb, program);
    obj_ref->name_ = sb->make_shared_string(name);
    return sb->emplace_object(obj_ref);
}
