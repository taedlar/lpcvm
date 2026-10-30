// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "code_generator.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"
#include "lpc_lexer.hpp"
#include "object_class.hpp"
#include <cstdio>
#include <stdexcept>

struct LpcCompiler::internal_s {
    LpcCompiler::Context ctx;
    yyscan_t scanner;
    yy::LpcParser* parser{nullptr};
};

LpcCompiler::LpcCompiler(FILE* source_file)
    : source_file_(source_file), impl_(nullptr)
{
    if (source_file_) {
        impl_ = new internal_s();
        yylex_init_extra(&impl_->ctx, &impl_->scanner); // create lexial scanner
        yyrestart(source_file_, impl_->scanner); // set the input for the scanner
        impl_->parser = new yy::LpcParser{impl_->scanner}; // create parser with the scanner
    }
    else {
        messages_.push_back({-1, -1, "No source file provided"});
    }
}

LpcCompiler::~LpcCompiler() {
    if (impl_) {
        if (impl_->scanner)
            yylex_destroy(impl_->scanner); // destroy the lexical scanner
        if (impl_->parser)
            delete impl_->parser; // destroy the parser instance
        delete impl_;
    }
}

bool LpcCompiler::init() {
    if (!impl_)
        return false;
    messages_.clear();
    consts_ = std::make_shared<ConstTable>();
    impl_->ctx.consts = consts_;
    globals_ = std::make_shared<SymbolTable>(SymbolTable::ScopeType::GLOBAL, 0, nullptr);
    impl_->ctx.current_scope = globals_;
    ast_.reset();
    return true;
}

bool LpcCompiler::parse() noexcept {
    bool result = false;
    try {
        if (!init())
            return false;
        impl_->parser->parse();
        ast_ = impl_->ctx.root;
        SPDLOG_INFO("parsing succeeded: consts={}, global_scope={}", consts_->size(), globals_->size());
        messages_.push_back({-1, -1, "parsing succeeded"});
        result = true;
    }
    catch (const yy::parse_error& e) {
        // Handle parsing errors
        SPDLOG_ERROR("parsing error at line {}, column {}: {}", e.lineno, e.column, e.what());
        messages_.push_back({e.lineno, e.column, e.what()});
    }
    catch (const std::exception& e) {
        // Handle other exceptions
        SPDLOG_ERROR("parse failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
    }
    return result;
}

/**
 * @brief Parses the given source code string.
 *
 * @param source The source code to parse.
 * @return true if parsing succeeded, false otherwise.
 */
bool LpcCompiler::parse (const std::string& source) noexcept {
    bool result = false;
    auto saved_impl = impl_;
    struct internal_s tmp;
    try {
        yylex_init_extra(&tmp.ctx, &tmp.scanner); // create lexial scanner
        auto bs = yy_scan_bytes(source.data(), source.size(), tmp.scanner);
        bs->yy_bs_lineno = 1; // initialize the line number for the buffer state
        bs->yy_bs_column = 0; // initialize the column number for the buffer state
        yy::LpcParser parser{tmp.scanner};
        tmp.parser = &parser; // use local parser instance (unwinding safe)
        impl_ = &tmp;
        result = parse(); // route the parsing through the main parse function
    }
    catch (const std::exception& e) {
        SPDLOG_ERROR("parse failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
    }
    if (tmp.scanner)
        yylex_destroy(tmp.scanner); // ensure the lexical scanner is destroyed even if an exception occurs
    impl_ = saved_impl; // restore the original impl_ to avoid dangling pointer
    return result;
}

std::shared_ptr<ObjectClass> LpcCompiler::generate_code() noexcept {
    // the AST must contain a valid program node
    if (!ast_ || std::holds_alternative<AST::Program>(ast_->node) == false) {
        messages_.push_back({-1, -1, "No valid AST available"});
        return nullptr;
    }
    AST::Program& prog_node = std::get<AST::Program>(ast_->node);

    std::shared_ptr<ObjectClass> prog; // the object representing the compiled program
    try {
        // compute number of global variables (for F_LOAD_GLOBAL / F_STORE_GLOBAL)
        std::size_t num_global_vars = globals_->size();
        for(auto& [key, entry]: globals_->symbols()) {
            if (entry->signature.b.is_const)
                --num_global_vars;
        }
        prog = std::make_shared<ObjectClass>(num_global_vars, consts_.get(), globals_.get());

        // compute number of functions (for F_CALL)
        std::size_t num_functions = globals_->size() - num_global_vars;
        prog->virtual_table_->static_bindings_.resize(num_functions);

        // generate code for each top-level definition in the program
        lpcvm::code_generator codegen(prog->bytecode_);
        for(auto& elt: prog_node.defs) {
            std::visit (overloaded {
                [&](AST::Inheritance& inh) {
                    // TODO: Implement inheritance handling (requires Sandbox integration)
                    // prog->inherit_from(std::make_shared<ObjectClass>());
                },
                [&](AST::FunctionDef& func_def) {
                    if (!func_def.body)
                        return; // forward declaration
                    AST::FunctionDecl& func_decl = std::get<AST::FunctionDecl>(func_def.decl->node);
                    auto name_idx = consts_->find(func_decl.entry->name);
                    if (name_idx == ConstPool::INVALID_INDEX)
                        throw std::runtime_error("Failed to find function name in constant pool");
                    auto func_idx = std::get<uint32_t>(*func_decl.entry->value);
                    if (func_idx >= prog->virtual_table_->static_bindings_.size())
                        throw std::runtime_error("Function index out of bounds");
                    lpcvm::function_builder func{codegen, name_idx, *prog->virtual_table_};
                    func.build(func_def);
                },
                [&](auto&) { /* ignore other node types */ }
            }, elt.node);
        }
    }
    catch (const std::exception& e) {
        SPDLOG_ERROR("code generation failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
    }
    return prog;
}
