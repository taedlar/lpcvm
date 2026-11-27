// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "console_module.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include "vm_executor.hpp"

ConsoleModule::ConsoleModule(std::shared_ptr<ManagedRuntime> fabric,
                             std::istream& input, std::ostream& output)
    : NativeModule(std::move(fabric)), input_(input), output_(output) {
    SymbolSignature signature{};
    signature.b.is_function = 1;
    signature.b.data_type = T_MIXED;
    add_method("read_line", signature, read_line);
    signature.b.data_type = T_INT;
    add_method("read_char", signature, read_char);
    signature.b.data_type = T_VOID;
    signature.b.num_formal_params = 1;
    add_method("write", signature, write);
    finalize_methods();
}

int ConsoleModule::read_line(NativeModule* self, lpcvm::DataStack* stack,
                             lpcvm::ReturnStack*, int) {
    auto& input = static_cast<ConsoleModule*>(self)->input_;
    std::string line;
    if (std::getline(input, line)) {
        stack->push_temp_string(line);
    } else if (input.eof() && !input.bad()) {
        stack->push(int32_t{0});
    } else {
        throw std::runtime_error("Console read_line failed");
    }
    return 1;
}

int ConsoleModule::read_char(NativeModule* self, lpcvm::DataStack* stack,
                             lpcvm::ReturnStack*, int) {
    auto& input = static_cast<ConsoleModule*>(self)->input_;
    const auto ch = input.get();
    if (ch == std::char_traits<char>::eof()) {
        if (!input.eof() || input.bad())
            throw std::runtime_error("Console read_char failed");
        stack->push(int32_t{-1});
    } else {
        stack->push(static_cast<int32_t>(static_cast<unsigned char>(ch)));
    }
    return 1;
}

int ConsoleModule::write(NativeModule* self, lpcvm::DataStack* stack,
                         lpcvm::ReturnStack*, int num_args) {
    auto& output = static_cast<ConsoleModule*>(self)->output_;
    const auto text = stack->top_string();
    output << text;
    output.flush();
    if (!output)
        throw std::runtime_error("Console write failed");
    stack->pop_n(num_args);
    return 0;
}
