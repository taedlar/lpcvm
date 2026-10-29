// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_types.hpp"
#include "object_class.hpp"
#include "object_state.hpp"
#include "vm_sandbox.hpp"

LpcObject::HandleType LpcObject::create(VMType *sb, std::string_view name, std::shared_ptr<ObjectClass> program) {
    auto obj_ref = lpcvm::make_shared<LpcObject>(program);
    obj_ref->name_ = std::string(name);
    if (auto it = sb->objects().find(name); it != sb->objects().end()) {
        sb->objects().erase(it); // also erase the string_view key for the old object
    }
    sb->objects().emplace(std::string_view(obj_ref->name_), obj_ref);
    return HandleType(obj_ref);
}

LpcObjectRef LpcObject::find(VMType *sb, std::string_view name) {
    auto it = sb->objects().find(name);
    if (it != sb->objects().end()) {
        return it->second;
    }
    return LpcObjectRef();
}

void LpcObject::destruct(VMType *sb, LpcObjectRef obj) {
    auto it = sb->objects().find(obj->name());
    if (it != sb->objects().end() && it->second == obj) {
        sb->objects().erase(it);
    }
}

LpcObject::LpcObject(std::shared_ptr<ObjectClass> program)
    : program_(std::move(program)), state_(std::make_shared<ObjectState>(program_->num_variables_inheriting())) {

}

