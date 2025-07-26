#pragma once
#include "common/internal/error.hpp"
#include "dsl/tokenizer.hpp"

#include <memory>
#include <string_view>
#include <variant>
#include <vector>

namespace statforge {

struct ExpressionTree; // fwd
using ExprPtr = std::unique_ptr<ExpressionTree>;

struct Literal {
    double value;
    Span span;
};
struct Ref {
    std::string_view name;
    Span span;
};

struct Unary {
    TokenKind op;
    ExprPtr rhs;
    Span span;
};
struct Binary {
    TokenKind op;
    ExprPtr lhs, rhs;
    Span span;
};
struct Ternary {
    ExprPtr cond, thenExpr, elseExpr;
    Span span;
};

struct Call {
    std::string_view name;
    std::vector<ExprPtr> args;
    Span span;
};

struct ExpressionTree : std::variant<Literal, Ref, Unary, Binary, Ternary, Call> {
    using std::variant<Literal, Ref, Unary, Binary, Ternary, Call>::variant;
};

std::string dumpSExpr(const ExpressionTree&);

} // namespace statforge