// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Ted Chang <taedlar@gmail.com>
#pragma once
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <vector>
#include "fabric.hpp"

namespace lpcvm::detail {

namespace fs = std::filesystem;

// Resolve each candidate before inspecting it so sandbox policy is applied even
// to directory entries which would not otherwise qualify as regular files.
template<class Resolve>
std::optional<PathType> search_file(
    const PathType& name,
    const std::vector<PathType>& directories, Resolve resolve) {
    const auto validate = [](const PathType& path) {
        if (path.native().find(PathType::value_type{}) !=
            PathType::string_type::npos)
            throw std::invalid_argument("File search paths cannot contain NULs");
    };
    if (name.empty())
        throw std::invalid_argument("File search name cannot be empty");
    validate(name);
    const auto check = [&](const PathType& candidate)
        -> std::optional<PathType> {
        try {
            auto [host, result] = resolve(candidate);
            if (fs::is_regular_file(host))
                return result;
        } catch (const fs::filesystem_error& error) {
            if (error.code() != std::errc::no_such_file_or_directory &&
                error.code() != std::errc::not_a_directory)
                throw;
        }
        return std::nullopt;
    };
    if (name.is_absolute())
        return check(name);
    for (const auto& directory : directories) {
        validate(directory);
        if (auto result = check(directory / name))
            return result;
    }
    return std::nullopt;
}

struct CanonicalObjectPath {
    PathType filesystem_path;
    PathType local_path;
};

// LPC absolute paths refer to the mudlib, never the host filesystem root.
CanonicalObjectPath canonicalize_object_path(const PathType &path, const PathType &mudlib_root);

} // namespace lpcvm::detail
