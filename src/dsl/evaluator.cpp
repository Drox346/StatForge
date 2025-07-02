#include "dsl/evaluator.hpp"
#include <cmath>
#include <stdexcept>

using namespace statforge;

namespace {

constexpr double trueD = 1.0;
constexpr double falseD = 0.0;

auto const logic = [](double const lhs, double const rhs, TokenKind const kind) -> double {
    bool const lhsBool{lhs != falseD};
    bool const rhsBool{rhs != falseD};

    switch (kind) {
    case TokenKind::AndAnd:
        return lhsBool && rhsBool ? trueD : falseD;
    case TokenKind::OrOr:
        return lhsBool || rhsBool ? trueD : falseD;
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
        throw std::logic_error{"unreachable"};
    }
};

auto const arithmetic = [](double const lhs, double const rhs, TokenKind const kind) -> double {
    switch (kind) {
    case TokenKind::Plus:
        return lhs + rhs;
    case TokenKind::Minus:
        return lhs - rhs;
    case TokenKind::Star:
        return lhs * rhs;
    case TokenKind::Slash:
        if (rhs == 0) {
            std::runtime_error{"divide by 0"};
        }
        return lhs / rhs;
    case TokenKind::Caret:
        return std::pow(lhs, rhs);
    default:
        throw std::logic_error{"unreachable"};
    }
};

} // namespace


double statforge::evaluate(ExpressionTree const& expression, Context const& context) {
    std::function<double(ExpressionTree const&)> const visit = [&](ExpressionTree const& node) -> double {
        return std::visit(
            [&](auto const& actual) -> double {
                using Node = std::decay_t<decltype(actual)>;

                if constexpr (std::is_same_v<Node, Literal>) {
                    return actual.value;
                }

                if constexpr (std::is_same_v<Node, Ref>) {
                    if (!context.cellLookup) {
                        throw std::runtime_error{"no cell-lookup callback provided"};
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
                        return rhs == 0.0 ? trueD : falseD;
                    default:
                        throw std::logic_error{"bad unary op"};
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
                        throw std::logic_error{"unknown binary op"};
                    }
                }

                if constexpr (std::is_same_v<Node, Ternary>) {
                    double const predicate{visit(*actual.cond)};
                    return predicate != 0.0 ? visit(*actual.thenExpr) : visit(*actual.elseExpr);
                }

                if constexpr (std::is_same_v<Node, Call>) {
                    if (actual.name == "root") {
                        if (actual.args.size() != 2U) {
                            throw std::runtime_error{"root() expects exactly two arguments"};
                        }
                        double const index{visit(*actual.args[0])};
                        double const value{visit(*actual.args[1])};
                        return std::pow(value, 1.0 / index);
                    }
                    throw std::runtime_error{"unknown function '" + std::string(actual.name) + '\''};
                }

                throw std::logic_error{"unhandled node type in visitor"};
            },
            node);
    };

    return visit(expression);
}

std::unordered_set<std::string_view> statforge::extractDependencies(ExpressionTree const& expression) {
    std::unordered_set<std::string_view> dependencies;
    std::vector<ExpressionTree const*> queue;
    queue.push_back(&expression);

    for (std::size_t cursor{0}; cursor < queue.size(); ++cursor) {
        ExpressionTree const* current{queue[cursor]};

        std::visit(
            [&](auto const& node) {
                using Node = std::decay_t<decltype(node)>;

                if constexpr (std::is_same_v<Node, Ref>) {
                    dependencies.insert(node.name);
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
    return dependencies;
}