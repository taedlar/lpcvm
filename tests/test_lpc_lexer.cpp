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
        // Flex does not initialize the line number for string buffers.
        yyset_lineno(0, scanner);
    }
};

TEST_F(LpcLexerTest, WhitespaceAndComments) {
    scan("first\n\n// comment\n/* comment\n * more\n */ second\n");
    EXPECT_EQ(yyget_lineno(scanner), 0);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_L_IDENTIFIER);
    EXPECT_EQ(yyget_lineno(scanner), 0);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_L_IDENTIFIER);
    EXPECT_EQ(yyget_lineno(scanner), 5);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_YYEOF);
    EXPECT_EQ(yyget_lineno(scanner), 6);
    yylex(scanner);
    EXPECT_EQ(yyget_lineno(scanner), 6);
}

TEST_F(LpcLexerTest, StringsCountSourceNewlinesOnly) {
    scan("\"escaped\\n\" \"continued\\\nline\\\r\nend\" R\"tag(raw\n)wrong\"\nend)tag\" after");
    EXPECT_EQ(yylex(scanner).value.as<std::string>(), "escaped\n");
    EXPECT_EQ(yyget_lineno(scanner), 0);
    EXPECT_EQ(yylex(scanner).value.as<std::string>(), "continuedlineend");
    EXPECT_EQ(yyget_lineno(scanner), 2);
    EXPECT_EQ(yylex(scanner).value.as<std::string>(), "raw\n)wrong\"\nend");
    EXPECT_EQ(yyget_lineno(scanner), 4);
    EXPECT_EQ(yylex(scanner).kind(), yy::LpcParser::symbol_kind::S_L_IDENTIFIER);
    EXPECT_EQ(yyget_lineno(scanner), 4);
}

TEST_F(LpcLexerTest, UnterminatedCommentPreservesLineAtEof) {
    scan("/* comment\nmore\n");
    EXPECT_THROW(yylex(scanner), yy::LpcParser::syntax_error);
    EXPECT_EQ(yyget_lineno(scanner), 2);
}

TEST_F(LpcLexerTest, InvalidStringCountsNewlineBeforeThrowing) {
    scan("\"invalid\n");
    EXPECT_THROW(yylex(scanner), yy::LpcParser::syntax_error);
    EXPECT_EQ(yyget_lineno(scanner), 1);
}
