#pragma once
#include "dsl/tokenizer.hpp"

#include <stdexcept>

namespace statforge {

struct DslError final : std::runtime_error {
    Span const span;

    DslError(Span spanIn, std::string const& what) : std::runtime_error{what}, span{spanIn} {
    }
};

} // namespace statforge