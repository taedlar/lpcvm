// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_compiler.hpp"
#include "symbol_table.hpp"
#include <gtest/gtest.h>

TEST(PreprocessorTest, Predefines) {
    auto predefines = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
    auto sym1 = std::make_shared<SymbolTable::Entry>("FOO", SymbolSignature{});
    sym1->value = "42";
    predefines->emplace("FOO", sym1);
    LpcCompiler compiler;
    std::string source = "int x = FOO;";
    compiler.use_predefines(predefines);
    std::string result = compiler.preprocess(source);
    SPDLOG_INFO("Preprocessed result: {}", result);
    EXPECT_NE(result.find("42"), std::string::npos);
}

TEST(PreprocessorTest, DefineUndefExpand) {
    LpcCompiler compiler;
    std::string result = compiler.preprocess(
        "#define A B + 1\n#define B 2\n#define S \"A\"\nint x = A; // A\nstring s = S;\n#undef B\nint y = B;\n");
    EXPECT_NE(result.find("int x = 2 + 1;"), std::string::npos) << result;
    EXPECT_NE(result.find("// A"), std::string::npos) << result;
    EXPECT_NE(result.find("string s = \"A\";"), std::string::npos) << result;
    EXPECT_NE(result.find("int y = B;"), std::string::npos) << result;
}
