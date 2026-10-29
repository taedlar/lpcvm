// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "const_pool.hpp"
#include "virtual_table.hpp"

class BytecodeView;

class ObjectClass {
    friend class LpcCompiler;
    std::vector<uint8_t> bytecode_;
    std::vector<std::shared_ptr<ObjectClass>> inherits_;
    std::shared_ptr<ConstPool> const_pool_;
    std::shared_ptr<VirtualTable> virtual_table_;
    std::size_t num_variables_; // (not including inherited variables)

public:
    struct SourceLocation {
        std::string_view source_file;
        std::size_t line_number = 0;
    };

    ObjectClass(std::size_t num_vars = 0,ConstTable* consts = nullptr, SymbolTable* globals = nullptr);
    ~ObjectClass() = default;

    std::size_t num_variables() const { return num_variables_; }
    std::size_t num_variables_inheriting() const {
        std::size_t total = num_variables_;
        for (const auto& parent : inherits_) {
            total += parent->num_variables_inheriting();
        }
        return total;
    }

    const uint8_t* bytecode() const { return bytecode_.data(); }
    const ConstPool* const_pool() const { return const_pool_.get(); }

    void set_source_location(std::size_t bytecode_offset, ConstPool::IndexType source_file, std::size_t line_number);
    std::optional<SourceLocation> lookup_source_location(const uint8_t* pc) const;

    bool lookup_method (const std::string_view name, BytecodeView* entry) const;
    const VirtualTable::Binding& get_method(std::size_t index) const;

    void inherit_from(std::shared_ptr<ObjectClass> parent);

private:
    std::map<std::size_t, SourceLocation> source_locations_;
};
