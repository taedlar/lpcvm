// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"
#include "lpc_lexer.hpp"
#include "vm_runtime_class.hpp"
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

    std::shared_ptr<ASTNode> parse (const std::string& source, ConstTable& consts, std::shared_ptr<SymbolTable> current_scope) {
        FILE* file = fmemopen((void*)source.c_str(), source.size(), "r");
        if (!file) {
            throw std::runtime_error("Failed to open memory stream for parsing");
        }

        // Reinitialize the scanner for the new parse session.
        if (scanner_) {
            yylex_destroy (scanner_);
            scanner_ = nullptr;
        }
        LpcCompiler::Context ctx;
        ctx.current_scope = current_scope;
        ctx.consts = &consts;
        yylex_init_extra (&ctx, &scanner_);
        yyrestart(file, scanner_);

        // Parse the input
        using namespace yy;
        LpcParser parser{scanner_};
        parser.parse();

        // Remember to close the memory stream after parsing
        fclose(file);
        return ctx.root;
    }

    int get_lineno() const { return yyget_lineno(scanner_); }
    int get_column() const { return yyget_column(scanner_); }

private:
    yyscan_t scanner_;
};

LpcCompiler::LpcCompiler() : pimpl_(new Impl()) {}
LpcCompiler::~LpcCompiler() = default;

ASTNode* LpcCompiler::frontend (const std::string& source) noexcept {
    try {
        consts_ = std::make_unique<ConstTable>();
        globals_ = std::make_shared<SymbolTable>(nullptr); // scoped
        messages_.clear();
        ast_ = pimpl_->parse(source, *consts_, globals_);
        SPDLOG_INFO("parsing succeeded: consts={}, global_scope={}", consts_->size(), globals_->size());
        messages_.push_back({-1, -1, "parsing succeeded"});
    }
    catch (const yy::parse_error& e) {
        // Handle parsing errors
        SPDLOG_ERROR("parsing error at line {}, column {}: {}", e.lineno, e.column, e.what());
        messages_.push_back({e.lineno, e.column, e.what()});
        return nullptr;
    }
    catch (const std::exception& e) {
        // Handle other exceptions
        SPDLOG_ERROR("failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
        return nullptr;
    }
    return ast_.get();
}
