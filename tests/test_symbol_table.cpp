// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "symbol_table.hpp"
#include <gtest/gtest.h>

TEST(SymbolTableTest, TokenIndicesSurviveGrowth) {
    SymbolTable table;
    const auto empty = table.new_token("");
    const auto first = table.new_token(std::string_view("a\0b", 3));
    std::vector<SymbolTable::TokenIndex> indices;
    for (int i = 0; i < 256; ++i)
        indices.push_back(table.new_token(std::string(256, 'x') + std::to_string(i)));

    EXPECT_TRUE(table.token_text(empty).empty());
    EXPECT_EQ(table.token_text(first), std::string_view("a\0b", 3));
    for (int i = 0; i < 256; ++i)
        EXPECT_EQ(table.token_text(indices[i]), std::string(256, 'x') + std::to_string(i));
}

TEST(SymbolTableTest, LookupReportsTokenOwner) {
    auto parent = std::make_shared<SymbolTable>();
    auto entry = std::make_shared<SymbolTable::Entry>("PARENT", SymbolSignature{});
    entry->value = parent->new_token("parent text");
    parent->emplace(entry->name, entry);
    auto child = parent->new_subscope(SymbolTable::ScopeType::BLOCK, 0);
    auto local = child->new_token("child text");

    const SymbolTable* owner = nullptr;
    auto found = child->lookup("PARENT", &owner);
    ASSERT_EQ(found, entry);
    ASSERT_EQ(owner, parent.get());
    EXPECT_EQ(owner->token_text(std::get<SymbolTable::TokenIndex>(*found->value)), "parent text");
    EXPECT_EQ(child->token_text(local), "child text");
    EXPECT_EQ(child->lookup("missing", &owner), nullptr);
    EXPECT_EQ(owner, nullptr);
}

TEST(SymbolTableTest, PushTokenPreservesChainAcrossStorageGrowth) {
    SymbolTable table;
    SymbolTable::Entry entry("F", SymbolSignature{});
    const auto first = table.new_token("a", SymbolTable::TokenType::MACRO_ARGUMENT);
    entry.push_token(table, first);
    for (int i = 0; i < 256; ++i)
        table.new_token(std::string(256, 'x'));
    const auto second = table.new_token("b", SymbolTable::TokenType::MACRO_ARGUMENT);
    entry.push_token(table, second);
    const auto body = table.new_token("a + b", SymbolTable::TokenType::MACRO_BODY);
    entry.push_token(table, body);

    ASSERT_TRUE(entry.value.has_value());
    EXPECT_EQ(std::get<SymbolTable::TokenIndex>(*entry.value), first);
    EXPECT_EQ(table.token(first).next, second);
    EXPECT_EQ(table.token(second).next, body);
    EXPECT_EQ(table.token(body).next, SymbolTable::invalid_token);
    EXPECT_EQ(table.token_text(first), "a");
    EXPECT_EQ(table.token_text(second), "b");
    EXPECT_EQ(table.token_text(body), "a + b");
}
