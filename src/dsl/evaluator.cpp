#include "dsl/evaluator.hpp"
#include "dsl/tokenizer.hpp"
#include "error/internal/error.hpp"
#include "types/definitions.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

using TokenKind = statforge::dsl::TokenKind;

namespace {

constexpr double trueD = 1.0;
constexpr double falseD = 0.0;

inline double logicalValue(double val) {
    return static_cast<double>((val != 0.0) && !std::isnan(val));
}

auto constexpr logic = [](double lhs, double rhs, TokenKind const kind) -> double {
    switch (kind) {
    case TokenKind::AndAnd:
        return logicalValue(lhs) * logicalValue(rhs);
    case TokenKind::OrOr:
        return std::max(logicalValue(lhs), logicalValue(rhs));
    case TokenKind::EqualEqual:
        return lhs == rhs ? trueD : falseD;
    case TokenKind::BangEqual:
        return lhs != rhs ? trueD : falseD;
    case TokenKind::Less:
        return lhs < rhs ? trueD : falseD;
    case TokenKind::LessEqual:
        return lhs <= rhs ? trueD : falseD;
    case TokenKind::Greater:
        return lhs > rhs ? trueD : falseD;
    case TokenKind::GreaterEqual:
        return lhs >= rhs ? trueD : falseD;
    default:
        statforge::unreachable(
            std::format("Invalid token in logic(): {}", std::to_underlying(kind)));
    }
};

auto constexpr arithmetic = [](double const lhs, double const rhs, TokenKind const kind) -> double {
    switch (kind) {
    case TokenKind::Plus:
        return lhs + rhs;
    case TokenKind::Minus:
        return lhs - rhs;
    case TokenKind::Star:
        return lhs * rhs;
    case TokenKind::Slash:
        //FIXME DIVIDE BY ZERO DETECTION std::fetestexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW)
        return lhs / rhs;
    case TokenKind::Caret:
        return std::pow(lhs, rhs);
    default:
        statforge::unreachable(
            std::format("Invalid token in arithmetic(): {}", std::to_underlying(kind)));
    }
};

} // namespace

namespace statforge::dsl {

double evaluate(ExpressionTree const& expression, Context const& context) {
    std::function<double(ExpressionTree const&)> const visit =
        [&](ExpressionTree const& node) -> double {
        return std::visit(
            [&](auto const& actual) -> double {
                using Node = std::decay_t<decltype(actual)>;

                if constexpr (std::is_same_v<Node, Literal>) {
                    return actual.value;
                }

                if constexpr (std::is_same_v<Node, Ref>) {
                    if (!context.cellLookup) {
                        unreachable("Missing cell-lookup callback");
                    }
                    return context.cellLookup(actual.name);
                }

                if constexpr (std::is_same_v<Node, Unary>) {
                    double const rhs{visit(*actual.rhs)};
                    switch (actual.op) {
                    case TokenKind::Plus:
                        return rhs;
                    case TokenKind::Minus:
                        return -rhs;
                    case TokenKind::Bang:
                        return logicalValue(rhs) == falseD ? trueD : falseD;
                    default:
                        statforge::unreachable(
                            std::format("Unknown unary op: {}", std::to_underlying(actual.op)));
                    }
                }

                if constexpr (std::is_same_v<Node, Binary>) {
                    double const lhs{visit(*actual.lhs)};
                    double const rhs{visit(*actual.rhs)};

                    switch (actual.op) {
                    case TokenKind::Plus:
                    case TokenKind::Minus:
                    case TokenKind::Star:
                    case TokenKind::Slash:
                    case TokenKind::Caret:
                        return arithmetic(lhs, rhs, actual.op);

                    case TokenKind::AndAnd:
                    case TokenKind::OrOr:
                    case TokenKind::EqualEqual:
                    case TokenKind::BangEqual:
                    case TokenKind::Less:
                    case TokenKind::LessEqual:
                    case TokenKind::Greater:
                    case TokenKind::GreaterEqual:
                        return logic(lhs, rhs, actual.op);

                    default:
                        statforge::unreachable(
                            std::format("Unknown binary op: {}", std::to_underlying(actual.op)));
                    }
                }

                if constexpr (std::is_same_v<Node, Ternary>) {
                    double const predicate{visit(*actual.cond)};
                    return logicalValue(predicate) == trueD ? visit(*actual.thenExpr)
                                                            : visit(*actual.elseExpr);
                }

                if constexpr (std::is_same_v<Node, Call>) {
                    if (actual.name == "root") {
                        if (actual.args.size() != 2U) {
                            unreachable("root() expects exactly two arguments, provided: " +
                                        std::to_string(actual.args.size()));
                        }
                        double const index{visit(*actual.args[0])};
                        double const value{visit(*actual.args[1])};
                        return std::pow(value, 1.0 / index);
                    }
                    statforge::unreachable(
                        std::format("Unknown function: {}", std::string(actual.name)));
                }

                statforge::unreachable("Unhandled node type in visitor");
            },
            node);
    };

    return visit(expression);
}

std::vector<CellId> extractDependencies(ExpressionTree const& expression) {
    std::vector<CellId> dependencies;
    std::vector<ExpressionTree const*> queue;
    queue.push_back(&expression);

    for (std::size_t cursor = 0; cursor < queue.size(); ++cursor) {
        ExpressionTree const* current{queue[cursor]};

        std::visit(
            [&](auto const& node) {
                using Node = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<Node, Ref>) {
                    dependencies.emplace_back(node.name);
                } else if constexpr (std::is_same_v<Node, Unary>) {
                    queue.push_back(node.rhs.get());
                } else if constexpr (std::is_same_v<Node, Binary>) {
                    queue.push_back(node.lhs.get());
                    queue.push_back(node.rhs.get());
                } else if constexpr (std::is_same_v<Node, Ternary>) {
                    queue.push_back(node.cond.get());
                    queue.push_back(node.thenExpr.get());
                    queue.push_back(node.elseExpr.get());
                } else if constexpr (std::is_same_v<Node, Call>) {
                    for (auto const& arg : node.args) {
                        queue.push_back(arg.get());
                    }
                }
            },
            *current);
    }

    std::ranges::sort(dependencies);
    auto dupes = std::ranges::unique(dependencies);
    dependencies.erase(dupes.begin(), dupes.end());
    return dependencies;
}

} // namespace statforge::dsl