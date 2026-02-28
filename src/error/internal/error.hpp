#pragma once

#include "error/error.h"

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

inline ErrorInfo buildErrorInfo(SF_ErrorCode code,
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

// The temporary "sf__tmp_err" enforces single evaluation
// and avoids double-move or other side effects.
#define SF_RETURN_ERROR_IF_UNEXPECTED(err)                                                         \
    do {                                                                                           \
        auto&& sf__tmp_err = (err);                                                                \
        if (!(sf__tmp_err)) [[unlikely]] {                                                         \
            return std::unexpected(std::move(sf__tmp_err).error());                                \
        }                                                                                          \
    } while (false)

#define SF_RETURN_UNEXPECTED_IF(condition, err, msg)                                               \
    do {                                                                                           \
        if ((condition)) [[unlikely]] {                                                            \
            return std::unexpected(buildErrorInfo((err), (msg)));                                  \
        }                                                                                          \
    } while (false)

#define SF_RETURN_UNEXPECTED_IF_SPAN(condition, err, msg, span)                                    \
    do {                                                                                           \
        if ((condition)) [[unlikely]] {                                                            \
            return std::unexpected(buildErrorInfo((err), (msg), (span)));                          \
        }                                                                                          \
    } while (false)

template <class T>
using Result = std::expected<T, ErrorInfo>;
using VoidResult = Result<void>;

[[noreturn]] inline void unreachable(
    std::string_view reason,
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
