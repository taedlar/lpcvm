// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"
#include "lpc_lexer.hpp"
#include <stdio.h>
#include <stdexcept>

// Wrapper class for the yyscan_t
class LpcCompiler::Impl {
public:
    Impl(): scanner(nullptr) {
    }
    ~Impl() {
        if (scanner) {
            yylex_destroy (scanner);
            scanner = nullptr;
        }
    }

    void parse (const std::string& source) {
        FILE* file = fmemopen((void*)source.c_str(), source.size(), "r");
        if (!file) {
            throw std::runtime_error("Failed to open memory stream for parsing");
        }

        // Reinitialize the scanner for the new parse session.
        if (scanner) {
            yylex_destroy (scanner);
            scanner = nullptr;
        }
        LpcCompiler::Context context;
        yylex_init_extra (&context, &scanner);
        yyrestart(file, scanner);

        // Parse the input
        using namespace yy;
        LpcParser parser{scanner};
        parser.parse();

        // Remember to close the memory stream after parsing
        fclose(file);
    }

    int get_lineno() const { return yyget_lineno(scanner); }
    int get_column() const { return yyget_column(scanner); }

private:
    yyscan_t scanner;
};

LpcCompiler::LpcCompiler() : pimpl(new Impl()) {}
LpcCompiler::~LpcCompiler() { delete pimpl; }

void LpcCompiler::compile(const std::string& source) {
    pimpl->parse(source);
}

int LpcCompiler::current_lineno() const {
    return pimpl->get_lineno();
}

int LpcCompiler::current_column() const {
    return pimpl->get_column();
}
