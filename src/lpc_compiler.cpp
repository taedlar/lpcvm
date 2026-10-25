// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#include "config.hpp"
#include "bytecode_generator.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "lpc_parser.hpp"
#include "lpc_lexer.hpp"
#include "runtime_class.hpp"
#include <cstdio>
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

    std::shared_ptr<ASTNode> parse_file (FILE* f, ConstTable& consts, std::shared_ptr<SymbolTable> current_scope) {
        // Reinitialize the scanner for the new parse session.
        if (scanner_) {
            yylex_destroy (scanner_);
            scanner_ = nullptr;
        }
        LpcCompiler::Context ctx;
        ctx.current_scope = current_scope;
        ctx.consts = &consts;
        yylex_init_extra (&ctx, &scanner_);
        yyrestart(f, scanner_);

        // Parse the input
        using namespace yy;
        LpcParser parser{scanner_};
        parser.parse();

        return ctx.root;
    }

    int get_lineno() const { return yyget_lineno(scanner_); }
    int get_column() const { return yyget_column(scanner_); }

private:
    yyscan_t scanner_;
};

LpcCompiler::LpcCompiler(FILE* source_file) : pimpl_(new Impl()), source_file_(source_file) {}
LpcCompiler::~LpcCompiler() = default;

bool LpcCompiler::parse() noexcept {
    bool result = false;
    try {
        consts_ = std::make_shared<ConstTable>();
        globals_ = std::make_shared<SymbolTable>(nullptr); // scoped
        messages_.clear();
        ast_ = pimpl_->parse_file(source_file_, *consts_, globals_);
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

bool LpcCompiler::parse (const std::string& source) noexcept {
    FILE* saved_source_file = source_file_;
    source_file_ = fmemopen((void*)source.c_str(), source.size(), "r");

    bool result = parse();

    fclose(source_file_);
    source_file_ = saved_source_file;
    return result;
}

std::shared_ptr<RuntimeClass> LpcCompiler::generate_code() noexcept {
    if (!ast_ || std::holds_alternative<AST::Program>(ast_->node) == false)
        return nullptr;
    try {
        // intern method names first.
        for (auto& elt: std::get<AST::Program>(ast_->node).defs) {
            if (std::holds_alternative<AST::FunctionDef>(elt.node)) {
                AST::FunctionDef& func_def = std::get<AST::FunctionDef>(elt.node);
                if (!func_def.body)
                    continue;
                AST::FunctionDecl& func_decl = std::get<AST::FunctionDecl>(func_def.decl->node);
                consts_->find_or_add(func_decl.entry->name);
            }
        }

        auto rt_class = std::make_shared<RuntimeClass>();
        rt_class->const_pool_ = consts_->make_intern();
        rt_class->virtual_table_ = std::make_shared<VirtualTable>(rt_class->const_pool_);
        globals_->emit_symbolic_bindings(rt_class->virtual_table_);
        rt_class->set_num_variables(globals_->size());
        for(auto& elt: std::get<AST::Program>(ast_->node).defs) {
            if (std::holds_alternative<AST::Inheritance>(elt.node)) {
                // TODO: Implement inheritance handling
                // rt_class->inherit_from(std::make_shared<RuntimeClass>());
            }
            else if (std::holds_alternative<AST::FunctionDef>(elt.node)) {
                // Extract the function declaration from the function definition
                AST::FunctionDef& func_def = std::get<AST::FunctionDef>(elt.node);
                if (!func_def.body)
                    continue; // Skip functions without a body
                AST::FunctionDecl& func_decl = std::get<AST::FunctionDecl>(func_def.decl->node);
                auto name_idx = consts_->find_or_add(func_decl.entry->name);
                std::size_t num_formal_params = std::get<AST::ParameterList>(func_decl.param_list->node).parameters.size();
                VirtualTable::Binding binding;
                binding.signature = func_decl.entry->signature;
                binding.inheriting = -1;
                // Generate the bytecode for the function body
                binding.bytecode_offset = rt_class->bytecode_.size();
                lpcvm::FunctionBuilder func_builder(rt_class->bytecode_);
                func_builder.emit_body(std::get<AST::Block>(func_def.body->node), num_formal_params);
                binding.bytecode_length = rt_class->bytecode_.size() - binding.bytecode_offset;
                // Add the method binding to the virtual table
                rt_class->virtual_table_->emplace(name_idx, binding);
            }
        }
        return rt_class;
    }
    catch (const std::exception& e) {
        SPDLOG_ERROR("code generation failed: {}", e.what());
        messages_.push_back({-1, -1, e.what()});
        return nullptr;
    }
}
