// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once

#include <iosfwd>
#include "native_module.hpp"

class ConsoleModule : public NativeModule {
public:
    // Streams must outlive the module. The shell supplies std::cin/std::cout.
    ConsoleModule(std::shared_ptr<ManagedRuntime> fabric, std::istream& input,
                  std::ostream& output);
    std::string_view name() const override { return "efun://console"; }

private:
    std::istream& input_;
    std::ostream& output_;

    static int read_line(NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int);
    static int read_char(NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int);
    static int write(NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int);
};
