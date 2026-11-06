// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "lpc_compiler.hpp"
#include "symbol_table.hpp"
#include "vm_sandbox.hpp"
#include <gtest/gtest.h>
#include <random>

namespace {
class IncludeTest : public ::testing::Test {
protected:
    std::filesystem::path root;
    void SetUp() override {
        std::random_device random;
        do {
            root = std::filesystem::temp_directory_path() /
                ("lpcvm-include-" + std::to_string(random()));
        } while (!std::filesystem::create_directory(root));
        std::filesystem::create_directories(root / "first/nested");
        std::filesystem::create_directory(root / "second");
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(root, error);
    }
    void write(const char* path, const std::string& text) {
        std::ofstream(root / path) << text;
    }
};
}

TEST_F(IncludeTest, SearchOrderReplacementAndBothDelimiters) {
    write("first/value.h", "#define VALUE 11\n");
    write("second/value.h", "#define VALUE 22\n");
    LpcCompiler compiler;
    compiler.use_include_directories({root / "first", root / "second"});
    EXPECT_NE(compiler.preprocess("#include <value.h>\nVALUE").find("11"), std::string::npos);
    EXPECT_TRUE(compiler.messages().empty());
    compiler.use_include_directories({root / "second"});
    EXPECT_NE(compiler.preprocess("#include\"value.h\"\nVALUE").find("22"), std::string::npos);
    EXPECT_TRUE(compiler.messages().empty());
    compiler.set_filename((root / "first/main.lpc").string());
    EXPECT_NE(compiler.preprocess("#include \"value.h\"\nVALUE").find("11"), std::string::npos);
    EXPECT_NE(compiler.preprocess("#include <value.h>\nVALUE").find("22"), std::string::npos);
}

TEST_F(IncludeTest, NestedIncludesShareMacrosAndResumeCaller) {
    write("first/outer.h", "#include \"nested/inner.h\"\n#undef BEFORE\n#define AFTER 23\n");
    write("first/nested/inner.h", "BEFORE\n#define INNER 19");
    LpcCompiler compiler;
    compiler.use_include_directories({root / "first"});
    const auto text = compiler.preprocess(
        "#define BEFORE 7\n#define HEADER \"outer.h\"\n"
        "#include HEADER /* comment */ // comment\nBEFORE AFTER INNER\n"
        "#include <nested/inner.h>");
    EXPECT_TRUE(compiler.messages().empty());
    EXPECT_NE(text.find("7\n"), std::string::npos) << text;
    EXPECT_NE(text.find("BEFORE 23 19"), std::string::npos) << text;
}

TEST_F(IncludeTest, ParsesStringAndFileSourcesWithIncludes) {
    write("first/value.h", "#define VALUE 42\nint included;\n");
    const std::string source = "#include <value.h>\nint answer() { return VALUE; }\n";
    write("main.lpc", source);
    LpcCompiler compiler;
    compiler.use_include_directories({root / "first"});
    ASSERT_TRUE(compiler.parse(source));
    EXPECT_TRUE(compiler.generate_code());
    const auto close_file = [](FILE* file) { std::fclose(file); };
    std::unique_ptr<FILE, decltype(close_file)> file(
        std::fopen((root / "main.lpc").string().c_str(), "r"), close_file);
    ASSERT_TRUE(file);
    LpcCompiler from_file(file.get());
    from_file.use_include_directories({root / "first"});
    ASSERT_TRUE(from_file.parse());
    EXPECT_TRUE(from_file.generate_code());
}

TEST_F(IncludeTest, ReportsErrorsAndCanBeReused) {
    write("first/cycle.h", "#include \"cycle.h\"\n");
    write("first/bad.h", "\n#define F(1) x\n");
    for (const char* directive : {"#include", "#include <>", "#include \"missing.h\"",
            "#include <bad.h> extra", "#include <cycle.h>", "#include <bad.h>"}) {
        for (bool parse : {false, true}) {
            LpcCompiler compiler;
            compiler.set_filename("main.lpc");
            compiler.use_include_directories({root / "first"});
            if (parse) EXPECT_FALSE(compiler.parse(std::string("\n") + directive));
            else EXPECT_TRUE(compiler.preprocess(std::string("\n") + directive).empty());
            ASSERT_EQ(compiler.messages().size(), 1u) << directive;
            EXPECT_EQ(compiler.messages()[0].lineno,
                      std::string(directive) == "#include <cycle.h>" ? 1 : 2) << directive;
            if (std::string(directive) == "#include <bad.h>")
                EXPECT_NE(compiler.messages()[0].text.find("bad.h:2:"), std::string::npos);
            EXPECT_EQ(compiler.preprocess("plain"), "plain");
            EXPECT_TRUE(compiler.messages().empty());
        }
    }
}

TEST_F(IncludeTest, SandboxFabricConfinesIncludesAndObjectLoading) {
    write("first/value.h", "#define VALUE 42\n");
    write("first/main.lpc", "#include \"value.h\"\nint answer() { return VALUE; }\n");
    write("second/secret.h", "secret");
    std::filesystem::create_symlink(root / "second/secret.h", root / "first/escape.h");
    auto sandbox = std::make_shared<lpcvm::Sandbox>(root / "first");
    LpcCompiler compiler;
    compiler.use_fabric(sandbox);
    compiler.use_include_directories({"/"});
    EXPECT_NE(compiler.preprocess("#include </value.h>\nVALUE").find("42"), std::string::npos);
    EXPECT_TRUE(compiler.messages().empty());
    for (const char* name : {"escape.h", "../second/secret.h"}) {
        EXPECT_TRUE(compiler.preprocess(std::string("#include <") + name + ">\n").empty());
        ASSERT_EQ(compiler.messages().size(), 1u);
    }
    EXPECT_TRUE(sandbox->load_object("main.lpc"));
}

TEST_F(IncludeTest, PreservesBoundariesCommentsAndLargeHeaders) {
    write("first/value.h", "TOKEN");
    write("first/large.h", std::string(20000, ' ') + "int included;\n");
    LpcCompiler compiler;
    compiler.use_include_directories({root / "first"});
    EXPECT_EQ(compiler.preprocess("#include <value.h>\nNEXT"),
              "#line 1 \"" + (root / "first/value.h").generic_string() +
              "\"\nTOKEN\n#line 1 \"<unknown>\"\n\nNEXT");
    const std::string comments = "/*\n#include <missing.h>\n*/\n// #include <missing.h>\n";
    EXPECT_EQ(compiler.preprocess(comments), comments);
    EXPECT_TRUE(compiler.parse("#include <large.h>\nint next;"));
}

TEST_F(IncludeTest, LimitsIncludeDepth) {
    for (int i = 0; i < 65; ++i) {
        const auto name = "first/header" + std::to_string(i) + ".h";
        write(name.c_str(), i == 64 ? "done\n" :
              "#include \"header" + std::to_string(i + 1) + ".h\"\n");
    }
    LpcCompiler compiler;
    compiler.use_include_directories({root / "first"});
    EXPECT_TRUE(compiler.preprocess("#include <header0.h>\n").empty());
    ASSERT_EQ(compiler.messages().size(), 1u);
    EXPECT_NE(compiler.messages()[0].text.find("Maximum #include depth exceeded"), std::string::npos);
    EXPECT_NE(compiler.preprocess("#include <header1.h>\n").find("done"), std::string::npos);
    EXPECT_TRUE(compiler.messages().empty());
}

TEST(PreprocessorTest, LineDirectiveMapsParserAndPreprocessorErrors) {
    for (const std::string newline : {"\n", "\r\n"}) {
        LpcCompiler compiler;
        compiler.set_filename("original.lpc");
        EXPECT_FALSE(compiler.parse("#line 40 \"virtual.lpc\"" + newline + "  @"));
        ASSERT_EQ(compiler.messages().size(), 1u);
        EXPECT_EQ(compiler.messages()[0].lineno, 40);
        EXPECT_EQ(compiler.messages()[0].column, 3);
        EXPECT_NE(compiler.messages()[0].text.find("virtual.lpc:40:3:"), std::string::npos);
        EXPECT_TRUE(compiler.preprocess("#line 80 \"virtual.lpc\"" + newline + "#define F(1) x").empty());
        ASSERT_EQ(compiler.messages().size(), 1u);
        EXPECT_EQ(compiler.messages()[0].lineno, 80);
        EXPECT_NE(compiler.messages()[0].text.find("virtual.lpc:80:"), std::string::npos);
        EXPECT_FALSE(compiler.parse("#define NUMBER 25\n#line NUMBER \"mapped.lpc\"\n#line 9\nint ;"));
        ASSERT_EQ(compiler.messages().size(), 1u);
        EXPECT_EQ(compiler.messages()[0].lineno, 9);
        EXPECT_NE(compiler.messages()[0].text.find("mapped.lpc:9:"), std::string::npos);
    }
}

TEST(PreprocessorTest, LineDirectiveValidatesOperandsAndPreservesLiterals) {
    for (const char* operand : {"", "0", "-1", "999999999999999999999", "12x",
                               "12 extra", "12 \"unterminated", "12 \"ok\" extra"}) {
        LpcCompiler compiler;
        EXPECT_TRUE(compiler.preprocess(std::string("\n#line ") + operand).empty());
        ASSERT_EQ(compiler.messages().size(), 1u) << operand;
        EXPECT_EQ(compiler.messages()[0].lineno, 2);
        EXPECT_NE(compiler.messages()[0].text.find("Invalid #line"), std::string::npos);
    }
    LpcCompiler compiler;
    EXPECT_EQ(compiler.preprocess("#line 7 \"a\\\\b\\\"c\" /* comment */"),
              "#line 7 \"a\\\\b\\\"c\"\n");
    EXPECT_FALSE(compiler.parse("#line 7 \"a\\\\b\\\"c\"\n@"));
    EXPECT_NE(compiler.messages()[0].text.find("a\\b\"c:7:1:"), std::string::npos);
    const std::string literal = "/*\n#line 90\n*/\n\"#line 20\"\n";
    EXPECT_EQ(compiler.preprocess(literal), literal);
}

TEST_F(IncludeTest, ParserDiagnosticsFollowNestedIncludesAndRestoreCaller) {
    write("first/outer.h", "\n#include \"nested/inner.h\"\nint outer;\n");
    write("first/nested/inner.h", "\n  @\n");
    LpcCompiler compiler;
    compiler.set_filename("main.lpc");
    compiler.use_include_directories({root / "first"});
    EXPECT_FALSE(compiler.parse("\n#include <outer.h>\nint main;\n"));
    ASSERT_EQ(compiler.messages().size(), 1u);
    EXPECT_NE(compiler.messages()[0].text.find("inner.h:2:3:"), std::string::npos);
    write("first/nested/inner.h", "int inner;");
    write("first/outer.h", "\n#include \"nested/inner.h\"\n  @");
    EXPECT_FALSE(compiler.parse("\n#include <outer.h>\nint main;\n"));
    EXPECT_NE(compiler.messages()[0].text.find("outer.h:3:3:"), std::string::npos);
    write("first/outer.h", "#include \"nested/inner.h\"");
    EXPECT_FALSE(compiler.parse("\n#include <outer.h>\n  @"));
    EXPECT_NE(compiler.messages()[0].text.find("main.lpc:3:3:"), std::string::npos);
}

TEST_F(IncludeTest, LogicalFilenamesDoNotChangeIncludeSearchDirectory) {
    write("first/outer.h", "#line 50 \"virtual/outer.h\"\n#include \"nested/inner.h\"\n@");
    write("first/nested/inner.h", "#line 100 \"virtual/inner.h\"\nint inner;\n");
    LpcCompiler compiler;
    compiler.use_include_directories({root / "first"});
    EXPECT_FALSE(compiler.parse("#include <outer.h>\n"));
    ASSERT_EQ(compiler.messages().size(), 1u);
    EXPECT_NE(compiler.messages()[0].text.find("virtual/outer.h:51:1:"), std::string::npos);
}

TEST_F(IncludeTest, AstLocationsRetainFilesAcrossLexerLookahead) {
    write("first/functions.h", "#line 30 \"virtual.h\"\nint header() { return 1; }\n");
    LpcCompiler compiler;
    compiler.set_filename("main.lpc");
    compiler.use_include_directories({root / "first"});
    ASSERT_TRUE(compiler.parse("#include <functions.h>\nint main() { return 2; }\n"));
    const auto& defs = std::get<AST::Program>(compiler.get_ast()->node).defs;
    ASSERT_EQ(defs.size(), 2u);
    const auto pool = compiler.get_consts()->freeze();
    const auto& header = *std::get<AST::FunctionDef>(defs[0].node).decl;
    const auto& main = *std::get<AST::FunctionDef>(defs[1].node).decl;
    EXPECT_EQ(pool->fetch_string(header.src.current_file), "virtual.h");
    EXPECT_EQ(header.src.lineno, 30);
    EXPECT_EQ(pool->fetch_string(main.src.current_file), "main.lpc");
    EXPECT_EQ(main.src.lineno, 2);
    EXPECT_TRUE(compiler.generate_code());
}

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
