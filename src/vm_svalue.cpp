// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_svalue.hpp"

std::mutex SValue::object_refs_mutex;
std::mutex SValue::mapping_refs_mutex;
std::mutex SValue::array_refs_mutex;

std::vector<std::weak_ptr<LpcObject>> SValue::all_object_refs;
std::vector<std::shared_ptr<LpcMapping>> SValue::all_mapping_refs;
std::vector<std::shared_ptr<LpcArray>> SValue::all_array_refs;

std::vector<LpcObjectRef> SValue::free_object_refs;
std::vector<LpcMappingRef> SValue::free_mapping_refs;
std::vector<LpcArrayRef> SValue::free_array_refs;
