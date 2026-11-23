// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include <argparse/argparse.hpp>
#include <cstdlib>
#include <filesystem>
#include "vm_executor.hpp"
#include "vm_sandbox.hpp"

int main (int argc, char* argv[]) {
    int log_level = spdlog::level::warn;
    argparse::ArgumentParser program("lpcshell", VERSION);

    program.add_argument("root").default_value("").help("The root directory of lpcshell sandbox");
    program.add_argument("--start").default_value("start.lpc").help("The startup object file to load");
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
        const std::string root_dir = program.get<std::string>("root");
        const std::string start_file = program.get<std::string>("--start");
        auto sandbox = std::make_shared<lpcvm::Sandbox>(root_dir);
        auto obj_ref = sandbox->load_object(start_file).lock();
        if (auto obj = dynamic_cast<LpcObject*>(&*obj_ref.value()); obj) {
            lpcvm::DataStack data_stack;
            lpcvm::ReturnStack return_stack;
            auto main_id = sandbox->make_shared_string("main");
            auto ret = obj->call_method(&*main_id, &data_stack, &return_stack, 0);
            SPDLOG_INFO("Method \"main\" returned: {}", ret);
        }
    } catch (const std::exception &e) {
        spdlog::error("{}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
