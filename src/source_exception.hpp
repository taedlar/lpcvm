#pragma once

#include "object_class.hpp"
#include <cstddef>
#include <exception>
#include <string>
#include <type_traits>

namespace lpcvm {

template <typename Exception>
class SourceException : public Exception {
    static_assert(std::is_base_of_v<std::exception, Exception>,
        "SourceException requires a standard exception type");

public:
    SourceException(const Exception& exception, const ObjectClass* prog, const uint8_t* pc)
        : SourceException(exception, prog ? prog->lookup_source_location(pc) : std::nullopt) {}

    const std::string& source_file() const noexcept { return source_file_; }
    std::size_t line_number() const noexcept { return line_number_; }

    const char* what() const noexcept override { return message_.c_str(); }

private:
    SourceException(const Exception& exception, const std::optional<ObjectClass::SourceLocation>& location)
                : Exception(exception), source_file_(location ? location->source_file : std::string_view{}),
                    line_number_(location ? location->line_number : 0),
                    message_(location ? source_file_ + ":" + std::to_string(line_number_)
              + ": " + exception.what() : exception.what()) {}

        std::string source_file_;
        std::size_t line_number_;
    std::string message_;
};

}