// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include <gtest/gtest.h>
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"
#include "lpc_lexer.hpp"

class LpcLexerTest : public ::testing::Test {
protected:
    LpcCompiler::Context context;
    yyscan_t scanner = nullptr;

    void SetUp() override {
        ASSERT_EQ(yylex_init_extra(&context, &scanner), 0);
    }

    void TearDown() override {
        yylex_destroy(scanner);
    }

    void scan(const char* source) {
        yy_scan_string(source, scanner);
    }
};

TEST_F(LpcLexerTest, WhitespaceAndComments) {
    scan("first\n\n// comment\n/* comment\n * more\n */ second\n");
    EXPECT_EQ(context.line_number, 1u);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_L_IDENTIFIER);
    EXPECT_EQ(context.line_number, 1u);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_L_IDENTIFIER);
    EXPECT_EQ(context.line_number, 6u);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_YYEOF);
    EXPECT_EQ(context.line_number, 7u);
    yylex(scanner);
    EXPECT_EQ(context.line_number, 7u);
}

TEST_F(LpcLexerTest, StringsCountSourceNewlinesOnly) {
    scan("\"escaped\\n\" \"continued\\\nline\\\r\nend\" R\"tag(raw\n)wrong\"\nend)tag\" after");
    EXPECT_EQ(yylex(scanner).value.as<std::string>(), "escaped\n");
    EXPECT_EQ(context.line_number, 1u);
    EXPECT_EQ(yylex(scanner).value.as<std::string>(), "continuedlineend");
    EXPECT_EQ(context.line_number, 3u);
    EXPECT_EQ(yylex(scanner).value.as<std::string>(), "raw\n)wrong\"\nend");
    EXPECT_EQ(context.line_number, 5u);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_L_IDENTIFIER);
    EXPECT_EQ(context.line_number, 5u);
}

TEST_F(LpcLexerTest, UnterminatedCommentPreservesLineAtEof) {
    scan("/* comment\nmore\n");
    EXPECT_THROW(yylex(scanner), yy::LpcParser::syntax_error);
    EXPECT_EQ(context.line_number, 3u);
}

TEST_F(LpcLexerTest, InvalidStringCountsNewlineBeforeThrowing) {
    scan("\"invalid\n");
    EXPECT_THROW(yylex(scanner), yy::LpcParser::syntax_error);
    EXPECT_EQ(context.line_number, 2u);
}
