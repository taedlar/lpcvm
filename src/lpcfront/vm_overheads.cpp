// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <spdlog/fmt/fmt.h>
#include "object_class.hpp"
#include "object_state.hpp"
#include "const_pool.hpp"
#include "virtual_table.hpp"
#include "vm_svalue.hpp"
#include "ast_node.hpp"
#include "symbol_table.hpp"

void write_vm_overheads() {
    std::cout << fmt::format("std::optional<...>      {:6d}", sizeof(std::optional<int>)) << std::endl;
    std::cout << fmt::format("std::unique_ptr<...>    {:6d}", sizeof(std::unique_ptr<ObjectClass>)) << std::endl;
    std::cout << fmt::format("std::shared_ptr<...>    {:6d}", sizeof(std::shared_ptr<ObjectClass>)) << std::endl;
    std::cout << fmt::format("std::weak_ptr<...>      {:6d}", sizeof(std::weak_ptr<ObjectClass>)) << std::endl;
    std::cout << fmt::format("std::string             {:6d}", sizeof(std::string)) << std::endl;
    std::cout << fmt::format("std::string_view        {:6d}", sizeof(std::string_view)) << std::endl;
    std::cout << fmt::format("std::vector<...>        {:6d}", sizeof(std::vector<uint8_t>)) << std::endl;
    std::cout << "# LPCVM Runtime" << std::endl;
    std::cout << fmt::format("SValue                  {:6d}", sizeof(SValue)) << std::endl;
    std::cout << fmt::format("ObjectClass             {:6d}", sizeof(ObjectClass)) << std::endl;
    std::cout << fmt::format("ObjectState             {:6d}", sizeof(ObjectState)) << std::endl;
    std::cout << fmt::format("ConstPool               {:6d}", sizeof(ConstPool)) << std::endl;
    std::cout << fmt::format("VirtualTable            {:6d}", sizeof(VirtualTable)) << std::endl;
    std::cout << "# Compile-time" << std::endl;
    std::cout << fmt::format("ASTNode                 {:6d}", sizeof(ASTNode)) << std::endl;
    std::cout << fmt::format("ConstTable              {:6d}", sizeof(ConstTable)) << std::endl;
    std::cout << fmt::format("ConstTable::ValueType   {:6d}", sizeof(ConstTable::ValueType)) << std::endl;
    std::cout << fmt::format("SymbolTable             {:6d}", sizeof(SymbolTable)) << std::endl;
    std::cout << fmt::format("SymbolTable::Entry      {:6d}", sizeof(SymbolTable::Entry)) << std::endl;
    std::cout << fmt::format("SymbolTable::Definition {:6d}", sizeof(SymbolTable::Definition)) << std::endl;
    std::cout << fmt::format("SymbolTable::Token      {:6d}", sizeof(SymbolTable::Token)) << std::endl;
}
