// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <argparse/argparse.hpp>
#include "lpc_compiler.hpp"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("lpcfront");

    program.add_argument("source")
        .help("The source file to compile");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }
    
    const std::string source_file = program.get<std::string>("source");
    LpcCompiler compiler;
    compiler.compile(source_file);
}
