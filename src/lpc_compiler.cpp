// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"
#include "lpc_lexer.hpp"
#include <stdio.h>
#include <stdexcept>

// Wrapper class for the lexer (yyscan_t) and parser (LpcParser)
class LpcCompiler::Impl {
public:
    Impl(): scanner_(nullptr) {
    }
    ~Impl() {
        if (scanner_) {
            yylex_destroy (scanner_);
            scanner_ = nullptr;
        }
    }

    void parse (const std::string& source) {
        FILE* file = fmemopen((void*)source.c_str(), source.size(), "r");
        if (!file) {
            throw std::runtime_error("Failed to open memory stream for parsing");
        }

        // Reinitialize the scanner for the new parse session.
        if (scanner_) {
            yylex_destroy (scanner_);
            scanner_ = nullptr;
        }
        context_ = std::make_unique<LpcCompiler::Context>();
        yylex_init_extra (context_.get(), &scanner_);
        yyrestart(file, scanner_);

        // Parse the input
        using namespace yy;
        LpcParser parser{scanner_};
        parser.parse();

        // Remember to close the memory stream after parsing
        fclose(file);
    }

    int get_lineno() const { return yyget_lineno(scanner_); }
    int get_column() const { return yyget_column(scanner_); }

private:
    yyscan_t scanner_;
    std::unique_ptr<LpcCompiler::Context> context_;
};

LpcCompiler::LpcCompiler() : pimpl_(new Impl()) {}
LpcCompiler::~LpcCompiler() { delete pimpl_; }

void LpcCompiler::compile(const std::string& source) {
    pimpl_->parse(source);
}

int LpcCompiler::current_lineno() const {
    return pimpl_->get_lineno();
}

int LpcCompiler::current_column() const {
    return pimpl_->get_column();
}
