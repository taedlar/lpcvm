// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <argparse/argparse.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ast_node.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "symbol_table.hpp"
#include "virtual_table.hpp"
#include "vm_runtime_class.hpp"
#include "vm_svalue.hpp"

static void log_vm_info();
static void ast_dump(ASTNode* ast, bool newline);

int main(int argc, char* argv[]) {
    int log_level = spdlog::level::warn;
    argparse::ArgumentParser program("lpcfront");

    program.add_argument("source")
        .help("The source file to compile");
    program.add_argument("-V", "--verbose").default_value(false).implicit_value(true).nargs(0)
        .action([&](const auto & /*unused*/) {
            if (log_level > spdlog::level::trace)
                log_level--;
        })
        .append() // -V: info, -VV: debug, -VVV: trace (debug/trace disabled in release builds)
        .help("increase verbosity of logging output");

    try {
        program.parse_args(argc, argv);
        spdlog::set_level(static_cast<spdlog::level::level_enum>(log_level));
        log_vm_info();
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    const std::string source_file = program.get<std::string>("source");
    std::ifstream input(source_file, std::ios::binary);
    if (!input) {
        std::cerr << "Failed to open source file: " << source_file << std::endl;
        return 1;
    }

    std::ostringstream source;
    source << input.rdbuf();
    if (input.bad() || source.bad()) {
        std::cerr << "Failed to read source file: " << source_file << std::endl;
        return 1;
    }

    LpcCompiler compiler;
    ASTNode* ast = compiler.frontend(source.str());
    if (ast) {
        ast_dump(ast, true);
        std::cout << std::endl;
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

static void log_vm_info() {
    SPDLOG_INFO("sizeof SValue {}", sizeof(SValue));
    SPDLOG_INFO("sizeof RuntimeClass {}", sizeof(RuntimeClass));
    SPDLOG_INFO("sizeof ConstPool {}", sizeof(ConstPool));
    SPDLOG_INFO("sizeof VirtualTable {}", sizeof(VirtualTable));
    SPDLOG_INFO("=======================================================");
    SPDLOG_INFO("sizeof ASTNode {}", sizeof(ASTNode));
    SPDLOG_INFO("sizeof ConstTable {}", sizeof(ConstTable));
    SPDLOG_INFO("sizeof ConstTable::ValueType {}", sizeof(ConstTable::ValueType));
    SPDLOG_INFO("sizeof ConstTable::LValueType {}", sizeof(ConstTable::LValueType));
    SPDLOG_INFO("sizeof SymbolTable {}", sizeof(SymbolTable));
    SPDLOG_INFO("sizeof SymbolTable::Entry {}", sizeof(SymbolTable::Entry));
    SPDLOG_INFO("=======================================================");
    SPDLOG_INFO("sizeof std::vector<...> {}", sizeof(std::vector<uint8_t>));
    SPDLOG_INFO("sizeof std::unique_ptr<...> {}", sizeof(std::unique_ptr<RuntimeClass>));
    SPDLOG_INFO("sizeof std::shared_ptr<...> {}", sizeof(std::shared_ptr<RuntimeClass>));
    SPDLOG_INFO("sizeof std::weak_ptr<...> {}", sizeof(std::weak_ptr<RuntimeClass>));
    SPDLOG_INFO("sizeof std::string_view {}", sizeof(std::string_view));
}

const char* no_sep = "";
const char* comma_sep = ",\n";

static void ast_dump(ASTNode* ast, bool newline) {
    static int indent_level = 0;
    const char* sep = no_sep;
    if (!ast)
        return;
    if (newline)
        std::cout << std::string(indent_level * 2, ' ');
    std::visit (overloaded {
        [&](AST::Program& program) {
            std::cout << "AST::Program([";
            indent_level++; sep = "\n";
            for (auto& def : program.defs) {
                std::cout << sep;
                ast_dump(&def, true);
                sep = comma_sep;
            }
            indent_level--;
            std::cout << "])";
        },
        [&](AST::Inheritance& inh) {
            std::cout << "AST::Inheritance(" << std::endl;
            indent_level++;
            if (inh.entry->value.has_value()) {
                std::cout << std::string(indent_level * 2, ' ') << "path = \"" << std::get<std::string>(*inh.entry->value) << "\"";
                if (inh.entry->name != std::get<std::string>(*inh.entry->value)) {
                    std::cout << comma_sep << std::string(indent_level * 2, ' ') << "alias = \"" << inh.entry->name << "\"";
                }
            }
            indent_level--;
            std::cout << ")";
        },
        [&](AST::FunctionDef& func) {
            std::cout << "AST::FunctionDef(" << std::endl;
            indent_level++;
            if (func.decl) {
                std::cout << std::string(indent_level * 2, ' ') << "decl = ";
                ast_dump(func.decl.get(), false);
            }
            if (func.body) {
                std::cout << comma_sep << std::string(indent_level * 2, ' ') << "body = ";
                ast_dump(func.body.get(), false);
            }
            indent_level--;
            std::cout << ")";
        },
        [&](AST::FunctionDecl& func_decl) {
            std::cout << "AST::FunctionDecl(" << std::endl;
            indent_level++;
            if (func_decl.entry) {
                std::cout << std::string(indent_level * 2, ' ') << "name = \"" << func_decl.entry->name << "\"";
            }
            if (func_decl.param_list) {
                std::cout << comma_sep << std::string(indent_level * 2, ' ') << "param_list = ";
                ast_dump(func_decl.param_list.get(), false);
            }
            indent_level--;
            std::cout << ")";
        },
        [&](AST::ParameterList& param_list) {
            std::cout << "AST::ParameterList([";
            indent_level++; sep = "\n";
            for (auto& param : param_list.parameters) {
                std::cout << sep;
                ast_dump(&param, true);
                sep = comma_sep;
            }
            indent_level--;
            std::cout << "])";
        },
        [&](AST::VariableDecl& var_decl) {
            std::cout << "AST::VariableDecl(";
            if (var_decl.entry) {
                std::cout << "name = \"" << var_decl.entry->name << "\"";
            }
            std::cout << ")";
        },
        [&](AST::Block& block) {
            std::cout << "AST::Block([";
            indent_level++; sep = "\n";
            for (auto& stmt : block.stmts) {
                std::cout << sep;
                ast_dump(&stmt, true);
                sep = comma_sep;
            }
            indent_level--;
            std::cout << "])";
        },
        [&](AST::PreTestedLoop& pre_tested_loop) {
            std::cout << "AST::PreTestedLoop(" << std::endl;
            indent_level++;
            if (pre_tested_loop.condition) {
                std::cout << std::string(indent_level * 2, ' ') << "condition = ";
                ast_dump(pre_tested_loop.condition.get(), false);
            }
            if (pre_tested_loop.step) {
                std::cout << comma_sep << std::string(indent_level * 2, ' ') << "step = ";
                ast_dump(pre_tested_loop.step.get(), false);
            }
            if (pre_tested_loop.body) {
                std::cout << comma_sep << std::string(indent_level * 2, ' ') << "body = ";
                ast_dump(pre_tested_loop.body.get(), false);
            }
            indent_level--;
            std::cout << ")";
        },
        [&](AST::PostTestedLoop& post_tested_loop) {
            std::cout << "AST::PostTestedLoop(" << std::endl;
            indent_level++;
            if (post_tested_loop.body) {
                std::cout << std::string(indent_level * 2, ' ') << "body = ";
                ast_dump(post_tested_loop.body.get(), false);
            }
            if (post_tested_loop.condition) {
                std::cout << comma_sep << std::string(indent_level * 2, ' ') << "condition = ";
                ast_dump(post_tested_loop.condition.get(), false);
            }
            indent_level--;
            std::cout << ")";
        },
        [&](AST::Return& ret_stmt) {
            std::cout << "AST::Return(";
            if (ret_stmt.value) {
                ast_dump(ret_stmt.value.get(), false);
            }
            std::cout << ")";
        },
        [&](AST::FunctionCall& func_call) {
            std::cout << "AST::FunctionCall(" << std::endl;
            indent_level++;
            std::cout << std::string(indent_level * 2, ' ') << "name = \"" << func_call.entry->name << "\"";
            indent_level--;
            std::cout << ")";
        },
        [&](auto&) {
            std::cout << "<NotImplemented>";
        }
    }, ast->node);
}
