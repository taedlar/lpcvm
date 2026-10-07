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
    Impl() {
        yylex_init_extra (&context, &scanner);
    }
    ~Impl() {
        yylex_destroy (scanner);
    }

    LpcCompiler::Context& get_context() {
        return context;
    }

    void parse (const std::string& source) {
        FILE* file = fmemopen((void*)source.c_str(), source.size(), "r");
        if (!file) {
            throw std::runtime_error("Failed to open memory stream for parsing");
        }
        context.line_number = 1;
        yyrestart(file, scanner);

        // Parse the input
        using namespace yy;
        LpcParser parser{scanner};
        parser.parse();

        // Remember to close the memory stream after parsing
        fclose(file);
    }

private:
    yyscan_t scanner;
    LpcCompiler::Context context;
};

LpcCompiler::LpcCompiler() : pimpl(new Impl()) {}
LpcCompiler::~LpcCompiler() { delete pimpl; }

void LpcCompiler::compile(const std::string& source) {
    pimpl->parse(source);
}

std::size_t LpcCompiler::get_line_number() const {
    return pimpl->get_context().line_number;
}
