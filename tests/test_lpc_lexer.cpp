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

TEST_F(LpcLexerTest, TokenLocationsAcrossComments) {
    scan("  alpha + 42;\n/* comment\n */\tif");
    const auto expect_token = [&](yy::LpcParser::symbol_kind_type kind, int line, int column, int end_column) {
        const auto token = yylex(scanner);
        EXPECT_EQ(token.kind(), kind);
        EXPECT_EQ(token.location.begin.line, line);
        EXPECT_EQ(token.location.begin.column, column);
        EXPECT_EQ(token.location.end.line, line);
        EXPECT_EQ(token.location.end.column, end_column);
    };
    expect_token(yy::LpcParser::symbol_kind::S_L_IDENTIFIER, 1, 3, 8);
    expect_token(yy::LpcParser::symbol_type('+', yy::location{}).kind(), 1, 9, 10);
    expect_token(yy::LpcParser::symbol_kind::S_L_INTEGER, 1, 11, 13);
    const auto semicolon = yylex(scanner);
    EXPECT_EQ(semicolon.location.begin.column, 13);
    expect_token(yy::LpcParser::symbol_kind::S_L_IF, 3, 5, 7);
    expect_token(yy::LpcParser::symbol_kind::S_YYEOF, 3, 7, 7);
    expect_token(yy::LpcParser::symbol_kind::S_YYEOF, 3, 7, 7);
}

TEST_F(LpcLexerTest, TokenLocationsPreserveFilename) {
    const std::string filename = "example.lpc";
    context.location.initialize(&filename);
    scan("first /* comment\n */ \"text\" R\"(raw\ntext)\"");
    for (const auto kind : {
        yy::LpcParser::symbol_kind::S_L_IDENTIFIER,
        yy::LpcParser::symbol_kind::S_L_STRING_LITERAL,
        yy::LpcParser::symbol_kind::S_L_STRING_LITERAL,
        yy::LpcParser::symbol_kind::S_YYEOF,
    }) {
        const auto token = yylex(scanner);
        EXPECT_EQ(token.kind(), kind);
        EXPECT_EQ(token.location.begin.filename, &filename);
        EXPECT_EQ(token.location.end.filename, &filename);
    }
}

TEST_F(LpcLexerTest, StringLocationsStartAtOpeningDelimiter) {
    scan("\"a\\n\" \"b\\\nc\" R\"tag(raw\n)wrong\"\nend)tag\" next");
    const auto expect_location = [&](int line, int column, int end_line, int end_column) {
        const auto token = yylex(scanner);
        EXPECT_EQ(token.location.begin.line, line);
        EXPECT_EQ(token.location.begin.column, column);
        EXPECT_EQ(token.location.end.line, end_line);
        EXPECT_EQ(token.location.end.column, end_column);
    };
    expect_location(1, 1, 1, 6);
    expect_location(1, 7, 2, 3);
    expect_location(2, 4, 4, 9);
    expect_location(4, 10, 4, 14);
    expect_location(4, 14, 4, 14);
}
