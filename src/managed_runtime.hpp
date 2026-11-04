// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <cstdio>
#include <string_view>
#include <unordered_map>
#include "lpc_types.hpp"

/** Default runtime services, usable without a Sandbox. Like SharedPtr, string
 * allocation, references, and sweeping are confined to the owning thread.
 */
class ManagedRuntime {
public:
    ManagedRuntime() = default;
    virtual ~ManagedRuntime() = default;

    virtual FILE* file_open(const char* path, const char* mode) {
        return std::fopen(path, mode);
    }

    /** Find or create an immutable owning string. Copies input bytes, including
     * embedded NULs. Returned strings can outlive this runtime.
     */
    virtual LpcStringRef make_shared_string(std::string_view value);

    /** Bytes allocated for strings currently interned here, including reference
     * counts, metadata, and the trailing NUL. Excludes table/allocator overhead.
     * The default implementation scans the table in linear time.
     */
    virtual std::size_t shared_string_allocation_size() const noexcept;

    /** Remove strings owned only by the table; return their allocation bytes.
     * Linear time by default. Buckets are retained; external weak handles may
     * delay physical deallocation. Overrides with their own storage should
     * override allocation accounting and sweeping together.
     */
    virtual std::size_t sweep_shared_strings();

private:
    static LpcStringRef allocate_shared_string(std::string_view value);
    std::unordered_map<std::string_view, LpcStringRef> strings_;
};
