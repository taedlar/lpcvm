// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "fabric.hpp"
#include "lpc_types.hpp"
#include "vm_svalue.hpp"
#include <memory>

using namespace lpcvm;

LpcArrayRef LpcArray::create(std::shared_ptr<FabricType> sb, std::size_t size) {
    return sb->make_container<LpcArrayRef>(size);
}

LpcArray::LpcArray(std::size_t size) : size_(size), capacity_(size) {
    if (size > std::numeric_limits<std::size_t>::max() / sizeof(svalue_t))
        throw std::bad_array_new_length();
    values_ = static_cast<svalue_t*>(std::malloc(sizeof(svalue_t) * size_));
    if (size_ && !values_)
        throw std::bad_alloc();
    for (std::size_t i = 0; i < size_; ++i) {
        new (values_ + i) svalue_t();
    }
}

LpcArray::~LpcArray() {
    for (std::size_t i = 0; i < size_; ++i) {
        values_[i].~svalue_t();
    }
    std::free(values_);
}

void LpcArray::resize(std::size_t new_size) {
    if (new_size <= capacity_) {
        for (std::size_t i = new_size; i < size_; ++i) {
            values_[i].~svalue_t();
        }
        for (std::size_t i = size_; i < new_size; ++i)
            new (values_ + i) svalue_t();
        size_ = new_size;
        return;
    }

    std::size_t new_capacity = new_size;
    if (new_capacity > std::numeric_limits<std::size_t>::max() / sizeof(svalue_t))
        throw std::bad_array_new_length();
    svalue_t* new_values = static_cast<svalue_t*>(std::malloc(sizeof(svalue_t) * new_capacity));
    if (!new_values)
        throw std::bad_alloc();
    for (std::size_t i = 0; i < size_; ++i) {
        new (new_values + i) svalue_t(std::move(values_[i]));
        values_[i].~svalue_t();
    }
    for (std::size_t i = size_; i < new_size; ++i)
        new (new_values + i) svalue_t();
    std::free(values_);
    values_ = new_values;
    size_ = new_size;
    capacity_ = new_capacity;
}

void LpcArray::clear() noexcept {
    for (std::size_t i = 0; i < size_; ++i) {
        values_[i].~svalue_t();
    }
    std::free(values_);
    values_ = nullptr;
    size_ = 0;
    capacity_ = 0;
}

void LpcArray::visit_edges(EdgeVisitor visitor, void* context) const noexcept {
    for (std::size_t i = 0; i < size_; ++i) {
        const auto& value = values_[i].value;
        if (const auto* array = std::get_if<LpcArrayRef>(&value); array && *array)
            visitor(&**array, context);
        else if (const auto* mapping = std::get_if<LpcMappingRef>(&value); mapping && *mapping)
            visitor(&**mapping, context);
    }
}

svalue_t& LpcArray::operator[](std::size_t index) noexcept {
    return values_[index];
}

const svalue_t& LpcArray::operator[](std::size_t index) const noexcept {
    return values_[index];
}
