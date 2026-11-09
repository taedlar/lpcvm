// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"
#include "lpc_lexer.hpp"
#include "preprocessor.hpp"
#include "object_class.hpp"
#include <cstdio>
#include <stdexcept>

struct LpcCompiler::internal_s {
    LpcCompiler::Context ctx;
    yyscan_t ppscanner{nullptr}; // preprocessor scanner handle
    yyscan_t scanner{nullptr}; // LPC lexical scanner handle
    yy::LpcParser* parser{nullptr}; // LPC parser instance
};

LpcCompiler::LpcCompiler(FILE* source_file)
    : source_file_(source_file), filename_("<unknown>"), impl_(nullptr)
{
    if (source_file_) {
        impl_ = new internal_s;
        pplex_init_extra(&impl_->ctx, &impl_->ppscanner); // create preprocessor scanner
        impl_->ctx.ppscanner = impl_->ppscanner;
        pprestart(source_file_, impl_->ppscanner); // the preprocessor reads the source file
        yylex_init_extra(&impl_->ctx, &impl_->scanner); // create lexial scanner
        yyrestart(nullptr, impl_->scanner); // the lexer reads from the preprocessor via YY_INPUT
        impl_->parser = new yy::LpcParser{impl_->scanner}; // create parser with the scanner
    }
    else {
        messages_.push_back({-1, -1, "No source file provided"});
    }
}

LpcCompiler::~LpcCompiler() {
    if (impl_) {
        if (impl_->ppscanner)
            pplex_destroy(impl_->ppscanner); // destroy the preprocessor scanner
        if (impl_->scanner)
            yylex_destroy(impl_->scanner); // destroy the lexical scanner
        if (impl_->parser)
            delete impl_->parser; // destroy the parser instance
        delete impl_;
    }
}

void LpcCompiler::use_fabric(std::shared_ptr<FabricType> sb) {
    if (sb == fabric_)
        return;
    fabric_ = std::move(sb);
    if (impl_)
        impl_->ctx.fabric = fabric_;
    ast_.reset();
    consts_.reset();
    globals_.reset();
}

void LpcCompiler::use_predefines(std::shared_ptr<SymbolTable> predefines) {
    predefines_ = std::move(predefines);
    if (predefines_ && impl_) {
        impl_->ctx.defines = predefines_;
        impl_->ctx.defines->new_subscope(SymbolTable::ScopeType::PREPROCESSOR, 0);
    }
}

void LpcCompiler::use_include_directories(std::vector<PathType> directories) {
    include_directories_ = std::move(directories);
}

bool LpcCompiler::init() {
    if (!impl_)
        return false;
    messages_.clear();
    if (!fabric_)
        fabric_ = std::make_shared<FabricType>();
    impl_->ctx.fabric = fabric_;
    impl_->ctx.include_directories = include_directories_;
    impl_->ctx.active_includes.clear();
    impl_->ctx.source_path = filename_ == "<unknown>" ? "" : filename_;
    consts_ = std::make_shared<ConstTable>(*fabric_);
    impl_->ctx.consts = consts_;
    if (!impl_->ctx.defines)
        impl_->ctx.defines = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
    globals_ = std::make_shared<SymbolTable>(SymbolTable::ScopeType::GLOBAL, 0, nullptr);
    impl_->ctx.location.initialize(&filename_);
    impl_->ctx.pp_location.initialize(&filename_);
    impl_->ctx.pp_eof = false;
    impl_->ctx.current_scope = globals_;
    if (!filename_.empty() && filename_ != "<unknown>") {
        impl_->ctx.current_file = consts_->find_or_add(filename_); // make an interned copy of current filename
    }
    ast_.reset();
    return true;
}

extern int preprocessor_feed (char* buf, int max_len, YY_EXTRA_TYPE lpcc);

std::string LpcCompiler::preprocess() noexcept {
    std::string result;
    try {
        if (!init())
            return result;
        char buffer[4096];
        int read;
        while ((read = preprocessor_feed(buffer, sizeof(buffer), &impl_->ctx)) > 0) {
            result.append(buffer, read);
        }
    }
    catch (const yy::LpcParser::syntax_error& e) {
        const auto& pos = e.location.begin;
        auto text = fmt::format("{}:{}:{}: {}", pos.filename ? *pos.filename : filename_, pos.line, pos.column, e.what());
        SPDLOG_DEBUG("preprocessing failed: {}", text);
        messages_.push_back({pos.line, pos.column, std::move(text)});
        return "";
    }
    catch (const std::exception& e) {
        SPDLOG_DEBUG("preprocessing failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
        return "";
    }
    return result;
}

std::string LpcCompiler::preprocess(const std::string& source) noexcept {
    std::string result;
    auto saved_impl = impl_;
    struct internal_s tmp;
    try {
        if (!fabric_)
            fabric_ = std::make_shared<FabricType>();
        tmp.ctx.fabric = fabric_;
        if (predefines_) {
            tmp.ctx.defines = predefines_;
            tmp.ctx.defines->new_subscope(SymbolTable::ScopeType::PREPROCESSOR, 0);
        }
        else
            tmp.ctx.defines = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
        pplex_init_extra(&tmp.ctx, &tmp.ppscanner); // create preprocessor scanner
        tmp.ctx.ppscanner = tmp.ppscanner;
        pp_scan_bytes(source.data(), source.size(), tmp.ppscanner); // feed the source string to the preprocessor
        impl_ = &tmp;
        result = preprocess();
    }
    catch (const std::exception& e) {
        SPDLOG_DEBUG("preprocessing failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
    }
    if (tmp.ppscanner)
        pplex_destroy(tmp.ppscanner);
    impl_ = saved_impl;
    return result;
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
        if (e.filename.empty()) {
            SPDLOG_DEBUG("parsing error at line {}, column {}: {}", e.lineno, e.column, e.what());
            messages_.push_back({e.lineno, e.column, e.what()});
        } else {
            auto text = fmt::format("{}:{}:{}: {}", e.filename, e.lineno, e.column, e.what());
            SPDLOG_DEBUG("parsing error: {}", text);
            messages_.push_back({e.lineno, e.column, std::move(text)});
        }
    }
    catch (const std::exception& e) {
        // Handle other exceptions
        SPDLOG_DEBUG("parse failed: {}", e.what());
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
        if (!fabric_)
            fabric_ = std::make_shared<FabricType>();
        tmp.ctx.fabric = fabric_;
        if (predefines_) {
            tmp.ctx.defines = predefines_;
            tmp.ctx.defines->new_subscope(SymbolTable::ScopeType::PREPROCESSOR, 0);
        }
        else
            tmp.ctx.defines = std::make_shared<SymbolTable>(SymbolTable::ScopeType::PREPROCESSOR, 0, nullptr);
        pplex_init_extra(&tmp.ctx, &tmp.ppscanner); // create preprocessor scanner
        tmp.ctx.ppscanner = tmp.ppscanner;
        pp_scan_bytes(source.data(), source.size(), tmp.ppscanner); // preprocessor reads the string
        yylex_init_extra(&tmp.ctx, &tmp.scanner); // create lexial scanner
        yyrestart(nullptr, tmp.scanner); // the lexer reads from the preprocessor via YY_INPUT
        yy::LpcParser parser{tmp.scanner};
        tmp.parser = &parser; // use local parser instance (unwinding safe)
        impl_ = &tmp;
        result = parse(); // route the parsing through the main parse function
    }
    catch (const std::exception& e) {
        SPDLOG_ERROR("parse failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
    }
    if (tmp.ppscanner)
        pplex_destroy(tmp.ppscanner);
    if (tmp.scanner)
        yylex_destroy(tmp.scanner); // ensure the lexical scanner is destroyed even if an exception occurs
    impl_ = saved_impl; // restore the original impl_ to avoid dangling pointer
    return result;
}

std::shared_ptr<ObjectClass> LpcCompiler::generate_code(bool string_accumulator) noexcept {
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
        lpcvm::Bytecode codegen(prog->bytecode_, prog.get(), string_accumulator);
        for(auto& elt: prog_node.defs) {
            std::visit (overloaded {
                [&](AST::Inheritance& inh) {
                    // TODO: Implement inheritance handling (requires Sandbox integration)
                    // prog->inherit_from(std::make_shared<ObjectClass>());
                },
                [&](AST::FunctionDef& func_def) {
                    if (!func_def.body)
                        return; // forward declaration
                    LpcCompiler::FunctionBuilder func(*this, codegen, *prog->virtual_table_);
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

void LpcCompiler::FunctionBuilder::build(const AST::FunctionDef& func_def) {
    // AST::FunctionDef contains the function declaration and body.
    // - The function index (assigned when parsing) is retrieved from the function name's
    //   symbol table entry associated with the function declaration.
    // - The number of formal parameters is determined from the function declaration's
    //   parameter list.
    const AST::FunctionDecl& func_decl = std::get<AST::FunctionDecl>(func_def.decl->node);
    auto name_idx = compiler_.get_consts()->find(func_decl.entry->name);
    if (name_idx == ConstPool::INVALID_INDEX)
        throw std::runtime_error("Failed to find function name in constant pool");
    auto func_idx = std::get<uint32_t>(*func_decl.entry->value);
    if (func_idx >= vtable_.static_bindings_.size())
        throw std::runtime_error("Function index out of bounds");
    std::size_t num_formal_params = std::get<AST::ParameterList>(func_decl.param_list->node).parameters.size();

    VirtualTable::Binding func_binding;
    func_binding.signature = func_decl.entry->signature;
    func_binding.inheriting = -1; // or some appropriate default value indicating no inheritance
    func_binding.bytecode_offset = codegen_.position(); // entry point of the function's bytecode

    // generate bytecode for the function body
    const AST::Block& body = std::get<AST::Block>(func_def.body->node);
    codegen_.emit_block(body, num_formal_params);
    func_binding.bytecode_length = codegen_.position() - func_binding.bytecode_offset;

    // register function bindings
    vtable_.bind(func_idx, func_binding);
    if (name_idx != ConstPool::INVALID_INDEX) {
        // TODO: Handle the case where the function has a name and should be added to the virtual table by name
        vtable_.emplace(name_idx, func_binding);
    }
}
