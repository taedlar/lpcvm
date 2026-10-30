#pragma once

#include "object_class.hpp"
#include <cstddef>
#include <exception>
#include <string>
#include <type_traits>
#include <spdlog/fmt/fmt.h>

namespace lpcvm {

template <typename Exception>
class SourceException : public Exception {
    static_assert(std::is_base_of_v<std::exception, Exception>,
        "SourceException requires a standard exception type");

public:
    SourceException(const Exception& exception, const ObjectClass* prog, const uint8_t* pc)
        : SourceException(exception, prog ? prog->lookup_source_location(pc) : std::nullopt) {}

    const std::string& source_file() const noexcept { return source_file_; }
    const std::string& func_name() const noexcept { return func_name_; }
    int lineno() const noexcept { return lineno_; }
    int column() const noexcept { return column_; }

    const char* what() const noexcept override { return message_.c_str(); }

private:
    SourceException(const Exception& exception, const std::optional<ObjectClass::SourceLocation>& location)
        : Exception(exception) {
        if (!location) {
            source_file_ = "<unknown-file>";
            func_name_ = "";
            lineno_ = 0; // a valid line number is 1-based
            column_ = 0; // a valid column is 1-based
            message_ = exception.what();
            return;
        }
        source_file_ = location->file_name;
        func_name_ = location->func_name;
        lineno_ = location->lineno;
        column_ = location->column;
        // format in GNU standard layout: "file:line:column: In function `func': message"
        message_ = fmt::format("{}:{}:{}: {}", source_file_, lineno_, column_,
            func_name_.empty() ? exception.what() : "In function `" + func_name_ + "()': " + exception.what());
    }

    std::string source_file_;
    std::string func_name_;
    int lineno_;
    int column_;
    std::string message_;
};

}