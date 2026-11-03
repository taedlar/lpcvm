// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_compiler.hpp"
#include "symbol_table.hpp"
#include <gtest/gtest.h>

TEST(PreprocessorTest, Predefines) {
    auto predefines = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
    auto sym1 = std::make_shared<SymbolTable::Entry>("FOO", SymbolSignature{});
    sym1->value = predefines->new_token("42", SymbolTable::TokenType::MACRO_BODY);
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

TEST(PreprocessorTest, MacroDefinitionsSurviveStorageGrowth) {
    LpcCompiler compiler;
    std::string source = "#define FIRST 42\n#define ALIAS FIRST\n";
    const std::string literal = "\"" + std::string(256, 'x') + "\"";
    source += "#define TEXT " + literal + "\n";
    for (int i = 0; i < 256; ++i)
        source += "#define M" + std::to_string(i) + " " + std::to_string(i) + "\n";
    source += "int x = ALIAS;\nstring s = TEXT;\n";
    for (int i = 0; i < 256; ++i)
        source += "int v" + std::to_string(i) + " = M" + std::to_string(i) + ";\n";

    const std::string result = compiler.preprocess(source);
    EXPECT_TRUE(compiler.messages().empty());
    EXPECT_NE(result.find("int x = 42;"), std::string::npos);
    EXPECT_NE(result.find("string s = " + literal + ";"), std::string::npos);
    for (int i = 0; i < 256; ++i) {
        SCOPED_TRACE(i);
        EXPECT_NE(result.find("int v" + std::to_string(i) + " = " + std::to_string(i) + ";"),
                  std::string::npos);
    }
}

TEST(PreprocessorTest, ContinuedDefinitions) {
    for (const std::string newline : {"\n", "\r\n"}) {
        SCOPED_TRACE(newline.size());
        const auto continued = "\\" + newline;
        LpcCompiler compiler;
        const auto result = compiler.preprocess(
            "#define VALUE 1 + " + continued +
            " 2 + " + continued + " 3" + newline +
            "#define ADD(a," + continued + " b) ((a) + " + continued + "(b))" + newline +
            "#define NAME VA" + continued + "LUE" + newline +
            "#define TEXT \"hel" + continued + "lo\"" + newline +
            "VALUE; ADD(NAME, 4); TEXT" + newline);
        EXPECT_TRUE(compiler.messages().empty());
        // Each consumed physical line still contributes a newline.
        EXPECT_EQ(result, std::string(10, '\n') +
                  "1 +  2 +  3; ((1 +  2 +  3) + (4)); \"hello\"" + newline);
    }
}

TEST(PreprocessorTest, ContinuedDefinitionAtEndOfInput) {
    auto definitions = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
    LpcCompiler compiler;
    compiler.use_predefines(definitions);
    EXPECT_EQ(compiler.preprocess("#define VALUE 4\\\n2"), "\n");
    EXPECT_TRUE(compiler.messages().empty());
    LpcCompiler consumer;
    consumer.use_predefines(definitions);
    EXPECT_EQ(consumer.preprocess("VALUE"), "42");
}

TEST(PreprocessorTest, ContinuedDefinitionsHaveSourceLocations) {
    for (bool parse : {false, true}) {
        for (const std::string source : {
                "#define F(a,\\\n 1) a",
                "#define F(a,\\\r\n 1) a",
                "#define VALUE 4\\\n2\n#define F(a,\\\n 1) a"}) {
            LpcCompiler compiler;
            if (parse) EXPECT_FALSE(compiler.parse(source));
            else compiler.preprocess(source);
            ASSERT_EQ(compiler.messages().size(), 1u);
            EXPECT_EQ(compiler.messages()[0].lineno, source.find("#define VALUE") == 0 ? 4 : 2);
            EXPECT_EQ(compiler.messages()[0].column, 2);
            EXPECT_NE(compiler.messages()[0].text.find("Invalid macro parameter"), std::string::npos);
        }
    }
}

TEST(PreprocessorTest, InvalidDefinitionsHaveSourceLocations) {
    struct Case { const char* directive; int column; const char* error; };
    const Case cases[] = {
        {"#define 123 value", 9, "Invalid macro name"},
        {"#define", 8, "Invalid macro name"},
        {"#define F(", 11, "Unterminated parameter list"},
        {"#define F(a", 12, "Unterminated parameter list"},
        {"#define F(a,   ", 16, "Unterminated parameter list"},
        {"#define F(1)", 11, "Invalid macro parameter"},
        {"#define F(a,)", 13, "Invalid macro parameter"},
        {"#define F(a b)", 13, "Expected ',' or ')'"},
        {"  # define F(a +)", 16, "Expected ',' or ')'"},
    };
    for (const auto& test : cases) {
        SCOPED_TRACE(test.directive);
        // A multiline comment and earlier expansion exercise original-source tracking.
        std::string source = std::string("/* comment\n */\n#define A 123456789\nint x = A;\n") + test.directive;
        for (bool parse : {false, true}) {
            LpcCompiler compiler;
            compiler.set_filename("macros.lpc");
            if (parse) EXPECT_FALSE(compiler.parse(source));
            else compiler.preprocess(source);
            ASSERT_EQ(compiler.messages().size(), 1u);
            const auto& message = compiler.messages().front();
            EXPECT_EQ(message.lineno, 5);
            EXPECT_EQ(message.column, test.column);
            EXPECT_NE(message.text.find(test.error), std::string::npos);
            EXPECT_NE(message.text.find("macros.lpc:5:" + std::to_string(test.column) + ":"), std::string::npos);
        }
    }
}

TEST(PreprocessorTest, ParameterListWhitespace) {
    LpcCompiler compiler;
    compiler.preprocess("#define F( a , b ) body\n#define G( ) body\n");
    EXPECT_TRUE(compiler.messages().empty());
}

TEST(PreprocessorTest, FunctionMacroArguments) {
    LpcCompiler compiler;
    const auto result = compiler.preprocess(
        "#define ADD(a,b) ((a)+(b))\n"
        "#define ID(x) x\n"
        "#define VALUE 42\n"
        "#define ZERO() VALUE\n"
        "ADD(1, ADD(2, 3))\nID(ID(VALUE))\nZERO () ZERO\n"
        "ID(call(1, 2))\nID(\"a, ) \\\" b\")\n"
        "ADD(\n 1 /* , ) */,\n 2)\nADD\n");
    EXPECT_TRUE(compiler.messages().empty());
    EXPECT_NE(result.find("((1)+(((2)+(3))))"), std::string::npos) << result;
    EXPECT_NE(result.find("\n42\n42 ZERO\n"), std::string::npos) << result;
    EXPECT_NE(result.find("call(1, 2)"), std::string::npos) << result;
    EXPECT_NE(result.find("\"a, ) \\\" b\""), std::string::npos) << result;
    EXPECT_NE(result.find("((1)+(2))\nADD\n"), std::string::npos) << result;
}

TEST(PreprocessorTest, FunctionMacroSubstitutionAndRescanning) {
    LpcCompiler compiler;
    const auto result = compiler.preprocess(
        "#define ID(x) x\n#define WRAP(x) ID(x)\n"
        "#define QUOTE(x) \"x\" xx x\n#define LOOP(x) LOOP(x)\n"
        "#define EMPTY()\n#define NOTHING\n"
        "WRAP(7)\nQUOTE(8)\nLOOP(9)\nEMPTY() NOTHING ID()\n");
    EXPECT_TRUE(compiler.messages().empty());
    EXPECT_NE(result.find("\n7\n\"x\" xx 8\nLOOP(9)\n  \n"), std::string::npos) << result;
}

TEST(PreprocessorTest, InvalidMacroInvocations) {
    for (const auto* invocation : {"F(1)", "F(1,2,3)", "F(1,", "F(\"unterminated"}) {
        LpcCompiler compiler;
        compiler.set_filename("arguments.lpc");
        compiler.preprocess(std::string("#define F(a,b) a+b\n  ") + invocation);
        ASSERT_EQ(compiler.messages().size(), 1u) << invocation;
        EXPECT_EQ(compiler.messages()[0].lineno, 2);
        EXPECT_EQ(compiler.messages()[0].column, 3);
        EXPECT_NE(compiler.messages()[0].text.find("macro argument"), std::string::npos);
    }
}

TEST(PreprocessorTest, DefinitionsStoreArgumentTokensBeforeBody) {
    auto definitions = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
    LpcCompiler compiler;
    compiler.use_predefines(definitions);
    compiler.preprocess("#define F(a,b) a + b\n#define ZERO() 42\n#define OBJECT 7\n");
    ASSERT_TRUE(compiler.messages().empty());
    auto entry = definitions->lookup("F");
    ASSERT_NE(entry, nullptr);
    ASSERT_TRUE(entry->value.has_value());
    auto index = std::get<SymbolTable::TokenIndex>(*entry->value);
    for (const auto* name : {"a", "b"}) {
        ASSERT_NE(index, SymbolTable::invalid_token);
        EXPECT_EQ(definitions->token(index).type, SymbolTable::TokenType::MACRO_ARGUMENT);
        EXPECT_EQ(definitions->token_text(index), name);
        index = definitions->token(index).next;
    }
    ASSERT_NE(index, SymbolTable::invalid_token);
    EXPECT_EQ(definitions->token(index).type, SymbolTable::TokenType::MACRO_BODY);
    EXPECT_EQ(definitions->token_text(index), "a + b");
    EXPECT_EQ(definitions->token(index).next, SymbolTable::invalid_token);
    for (const auto* name : {"ZERO", "OBJECT"}) {
        auto macro = definitions->lookup(name);
        ASSERT_NE(macro, nullptr);
        ASSERT_TRUE(macro->value.has_value());
        auto body = std::get<SymbolTable::TokenIndex>(*macro->value);
        EXPECT_EQ(definitions->token(body).type, SymbolTable::TokenType::MACRO_BODY);
        EXPECT_EQ(definitions->token(body).next, SymbolTable::invalid_token);
        EXPECT_EQ(macro->signature.b.is_function, std::string_view(name) == "ZERO");
    }

    // Resolve the stored chain through its owner, even with unrelated local tokens.
    auto child = definitions->new_subscope(SymbolTable::ScopeType::PREPROCESSOR, 0);
    child->new_token("unrelated");
    LpcCompiler consumer;
    consumer.use_predefines(child);
    EXPECT_EQ(consumer.preprocess("F(ZERO(), OBJECT)"), "42 + 7");
    EXPECT_TRUE(consumer.messages().empty());
}
