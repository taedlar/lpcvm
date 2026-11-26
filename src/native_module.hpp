// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>

#pragma once
#include <algorithm>
#include <cstddef>
#include <functional>
#include <string_view>
#include <stdexcept>
#include <utility>
#include <vector>
#include "lpc_types.hpp"
#include "symbol_table.hpp"
#include "vm_object.hpp"
#include "vm_sandbox.hpp"

class NativeModule: public lpcvm::Object {
public:
    using NativeMethod = int (*)(NativeModule*, lpcvm::DataStack*, lpcvm::ReturnStack*, int);

    explicit NativeModule(std::shared_ptr<FabricType> sb)
        : lpcvm::Object(std::move(sb)) {}
    virtual ~NativeModule() = default;

    // Register methods during sandbox/module initialization.
    // Call finalize_methods() once after registration.
    void add_method(std::string_view name, SymbolSignature signature, NativeMethod method);

    struct ExportedMethod {
        LpcStringRef name;
        SymbolSignature signature{};
    };

    // Export finalized tables only. Imports must precede this module's finalization,
    // so source dependencies cannot form cycles. Imported methods are re-exported.
    std::vector<ExportedMethod> export_methods() const;
    void import_method(lpcvm::SharedPtr<NativeModule> source, std::string_view name);

    void finalize_methods();

    int call_method(void* func_identity, lpcvm::DataStack* data_stack,
                    lpcvm::ReturnStack* return_stack, int num_args) override;

private:
    struct MethodEntry {
        LpcStringRef name;
        SymbolSignature signature{};
        NativeMethod method = nullptr;
        // For imports, retain the module that originally registered method.
        lpcvm::SharedPtr<NativeModule> source;

        const LpcString* identity() const noexcept {
            return name ? std::addressof(*name) : nullptr;
        }
    };

    bool finalized_ = false;
    std::vector<MethodEntry> methods_;
};
