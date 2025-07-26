#pragma once

#include "common/error.h"

#include <cassert>
#include <expected>
#include <optional>
#include <print>
#include <source_location>
#include <string>
#include <utility>

namespace statforge {

struct Span {
    std::size_t line{1}, column{1};
};

struct ErrorInfo {
    SF_ErrorCode errorCode;
    std::string message;
    std::optional<Span> span;
    std::string functionName;
};

constexpr ErrorInfo buildErrorInfo(SF_ErrorCode code,
                                   std::string msg,
                                   std::optional<Span> span = std::nullopt,
                                   std::source_location loc = std::source_location::current()) {
    return {
        .errorCode = code,
        .message = std::move(msg),
        .span = span,
        .functionName = loc.function_name(),
    };
}

template <class T>
using Result = std::expected<T, ErrorInfo>;
using VoidResult = Result<void>;

[[noreturn]] inline void unreachable(std::string_view reason,
                                     std::source_location loc = std::source_location::current()) noexcept {
#ifndef NDEBUG
    std::println(stderr,
                 "UNREACHABLE at {}:{} ({}): {:.{}}",
                 loc.file_name(),
                 loc.line(),
                 loc.function_name(),
                 reason.data(),
                 int(reason.size()));
    assert(false && "unreachable() hit");
#endif
    std::terminate();
    std::unreachable();
}

} // namespace statforge
