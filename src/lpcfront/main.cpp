// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <argparse/argparse.hpp>
#include <fstream>
#include <sstream>
#include "ast_node.hpp"
#include "const_pool.hpp"
#include "lpc_compiler.hpp"
#include "symbol_table.hpp"
#include "vm_runtime_class.hpp"
#include "virtual_table.hpp"

static void log_vm_info();

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
    return compiler.compile(source.str()) ? 0 : 1;
}

static void log_vm_info() {
    SPDLOG_INFO("sizeof RuntimeClass {}", sizeof(RuntimeClass));
    SPDLOG_INFO("sizeof ConstPool {}", sizeof(ConstPool));
    SPDLOG_INFO("sizeof VirtualTable {}", sizeof(VirtualTable));
    SPDLOG_INFO("=======================================================");
    SPDLOG_INFO("sizeof ASTNode {}", sizeof(ASTNode));
    SPDLOG_INFO("sizeof ConstTable {}", sizeof(ConstTable));
    SPDLOG_INFO("sizeof ConstTable::ValueType {}", sizeof(ConstTable::ValueType));
    SPDLOG_INFO("sizeof SymbolTable {}", sizeof(SymbolTable));
    SPDLOG_INFO("sizeof SymbolTable::Entry {}", sizeof(SymbolTable::Entry));
    SPDLOG_INFO("=======================================================");
    SPDLOG_INFO("sizeof std::vector<...> {}", sizeof(std::vector<uint8_t>));
    SPDLOG_INFO("sizeof std::unique_ptr<...> {}", sizeof(std::unique_ptr<RuntimeClass>));
    SPDLOG_INFO("sizeof std::shared_ptr<...> {}", sizeof(std::shared_ptr<RuntimeClass>));
    SPDLOG_INFO("sizeof std::string_view {}", sizeof(std::string_view));
}
