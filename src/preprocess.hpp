// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <climits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace pp {
struct LineDirective {
    int line;
    std::optional<std::string> filename;
};

inline LineDirective parse_line_directive(std::string_view text) {
    auto space = [&] {
        while (!text.empty() && (text.front() == ' ' || text.front() == '\t' ||
               text.front() == '\r' || text.front() == '\n')) text.remove_prefix(1);
    };
    auto fail = [] { throw std::invalid_argument("Invalid #line directive"); };
    space();
    int line = 0;
    while (!text.empty() && text.front() >= '0' && text.front() <= '9') {
        const int digit = text.front() - '0';
        if (line > (INT_MAX - digit) / 10) fail();
        line = line * 10 + digit;
        text.remove_prefix(1);
    }
    if (!line) fail();
    if (!text.empty() && text.front() != ' ' && text.front() != '\t' &&
        text.front() != '\r' && text.front() != '\n') fail();
    space();
    std::optional<std::string> filename;
    if (!text.empty() && text.front() == '"') {
        text.remove_prefix(1);
        filename.emplace();
        while (!text.empty() && text.front() != '"') {
            char c = text.front();
            text.remove_prefix(1);
            if (c == '\\') {
                if (text.empty() || (text.front() != '\\' && text.front() != '"')) fail();
                c = text.front();
                text.remove_prefix(1);
            }
            if (c == '\n' || c == '\r' || c == '\0') fail();
            filename->push_back(c);
        }
        if (text.empty()) fail();
        text.remove_prefix(1);
        space();
    }
    while (text.substr(0, 2) == "/*") {
        const auto end = text.find("*/", 2);
        if (end == std::string_view::npos) fail();
        text.remove_prefix(end + 2);
        space();
    }
    if (!text.empty() && text.substr(0, 2) != "//") fail();
    return {line, std::move(filename)};
}

inline std::string line_marker(int line, const std::string& filename) {
    std::string result = "#line " + std::to_string(line) + " \"";
    for (char c : filename) {
        if (c == '\\' || c == '"') result += '\\';
        if (c == '\n' || c == '\r' || c == '\0')
            throw std::invalid_argument("Filename cannot be represented in #line");
        result += c;
    }
    return result + "\"\n";
}
} // namespace pp
