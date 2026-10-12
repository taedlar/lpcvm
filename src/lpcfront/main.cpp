// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <argparse/argparse.hpp>
#include <fstream>
#include <sstream>
#include "lpc_compiler.hpp"

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
    SPDLOG_INFO("sizeof std::string_view {}", sizeof(std::string_view));
}
