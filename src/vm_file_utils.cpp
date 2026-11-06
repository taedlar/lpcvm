// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#include "config.hpp"
#include "vm_file_utils.hpp"
#include <utility>

using namespace lpcvm::detail;

std::optional<PathType> ManagedRuntime::file_search(
    const PathType& name,
    const std::vector<PathType>& search_paths) {
    return search_file(name, search_paths, [](const auto& candidate) {
        auto resolved = fs::canonical(candidate);
        return std::make_pair(resolved, resolved);
    });
}

namespace lpcvm::detail {
CanonicalObjectPath canonicalize_object_path(const PathType &path, const PathType &mudlib_root) {
    const auto root = fs::canonical(mudlib_root);
    const auto requested = root / path.relative_path();
    const auto resolved = fs::canonical(requested);

    // Compare components so a sibling named "mudlib-other" is excluded.
    auto candidate = resolved.begin();
    for (const auto& component : root) {
        if (candidate == resolved.end() || *candidate != component) {
            throw fs::filesystem_error(
                "Access denied", resolved,
                std::make_error_code(std::errc::permission_denied));
        }
        ++candidate;
    }
    return {resolved, resolved.lexically_relative(root)};
}
} // namespace lpcvm::detail
