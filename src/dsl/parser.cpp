#include "dsl/parser.hpp"
#include "common/error.h"
#include "common/internal/error.hpp"
#include "dsl/tokenizer.hpp"

#include <cmath>
#include <expected>

namespace statforge {

VoidResult Parser::verify(ExprPtr const& /*ast*/) {
    return {};
}

const Token& Parser::peek(std::size_t offset) const {
    return _tokens[_pos + offset];
}

bool Parser::match(TokenKind kind) {
    if (peek().kind != kind) {
        return false;
    }
    ++_pos;
    return true;
}

const Token& Parser::advance() {
    return _tokens[_pos++];
}

Parser::BindingPower Parser::leftBindingPower(TokenKind kind) {
    switch (kind) {
    case TokenKind::Caret:
        return 11; // ^
    case TokenKind::Star:
    case TokenKind::Slash:
        return 9; // * /
    case TokenKind::Plus:
    case TokenKind::Minus:
        return 8; // + -
    case TokenKind::Less:
    case TokenKind::LessEqual:
    case TokenKind::Greater:
    case TokenKind::GreaterEqual:
    case TokenKind::EqualEqual:
    case TokenKind::BangEqual:
        return 7;
    case TokenKind::AndAnd:
        return 6;
    case TokenKind::OrOr:
        return 5;
    default:
        return -1;
    }
}
Parser::BindingPower Parser::rightBindingPower(TokenKind kind) {
    // left-assoc: rbp = lbp + 1 ; right-assoc: rbp = lbp
    switch (kind) {
    case TokenKind::Caret:
        return 11;
    case TokenKind::Star:
    case TokenKind::Slash:
        return 10;
    case TokenKind::Plus:
    case TokenKind::Minus:
        return 9;
    case TokenKind::Less:
    case TokenKind::LessEqual:
    case TokenKind::Greater:
    case TokenKind::GreaterEqual:
    case TokenKind::EqualEqual:
    case TokenKind::BangEqual:
        return 8;
    case TokenKind::AndAnd:
        return 7;
    case TokenKind::OrOr:
        return 6;
    default:
        return -1;
    }
}

ExpressionPtrResult Parser::foldConstants(ExprPtr node) {
    auto lit = [](double val, Span span) {
        return std::make_unique<ExpressionTree>(Literal{.value = val, .span = span});
    };

    std::optional<ErrorInfo> error;

    std::visit(
        [&](auto& actual) {
            using Node = std::decay_t<decltype(actual)>;

            auto prop = [&](ExpressionPtrResult r) {
                if (!r) {
                    error.emplace(std::move(r).error());
                    return ExprPtr{};
                }
                return std::move(r).value();
            };

            if constexpr (std::is_same_v<Node, Unary>) {
                actual.rhs = prop(foldConstants(std::move(actual.rhs)));
                if (error) {
                    return;
                }
                if (auto const* rhs = std::get_if<Literal>(actual.rhs.get())) {
                    double val = (actual.op == TokenKind::Minus)  ? -rhs->value
                                 : (actual.op == TokenKind::Plus) ? rhs->value
                                                                  : (rhs->value == 0.0 ? 1.0 : 0.0);
                    node = lit(val, actual.span);
                }

            } else if constexpr (std::is_same_v<Node, Binary>) {
                actual.lhs = prop(foldConstants(std::move(actual.lhs)));
                if (error) {
                    return;
                }
                actual.rhs = prop(foldConstants(std::move(actual.rhs)));
                if (error) {
                    return;
                }
                if (auto const* lhs = std::get_if<Literal>(actual.lhs.get()))
                    if (auto const* rhs = std::get_if<Literal>(actual.rhs.get())) {
                        if (actual.op == TokenKind::Slash && rhs->value == 0.0) {
                            error.emplace(
                                buildErrorInfo(SF_ERR_INVALID_DSL, "Division by zero not allowed", actual.span));
                            return;
                        }
                        double val{};
                        switch (actual.op) {
                        case TokenKind::Plus:
                            val = lhs->value + rhs->value;
                            break;
                        case TokenKind::Minus:
                            val = lhs->value - rhs->value;
                            break;
                        case TokenKind::Star:
                            val = lhs->value * rhs->value;
                            break;
                        case TokenKind::Slash:
                            val = lhs->value / rhs->value;
                            break;
                        case TokenKind::Caret:
                            val = std::pow(lhs->value, rhs->value);
                            break;
                        default:
                            return; // don't fold logical comps
                        }
                        node = lit(val, actual.span);
                    }

            } else if constexpr (std::is_same_v<Node, Ternary>) {
                actual.cond = prop(foldConstants(std::move(actual.cond)));
                if (error) {
                    return;
                }
                actual.thenExpr = prop(foldConstants(std::move(actual.thenExpr)));
                if (error) {
                    return;
                }
                actual.elseExpr = prop(foldConstants(std::move(actual.elseExpr)));
                if (error) {
                    return;
                }

                if (auto const* c = std::get_if<Literal>(actual.cond.get())) {
                    node = (c->value != 0.0) ? std::move(actual.thenExpr) : std::move(actual.elseExpr);
                }

            } else if constexpr (std::is_same_v<Node, Call>) {
                for (auto& arg : actual.args) {
                    arg = prop(foldConstants(std::move(arg)));
                    if (error) {
                        return;
                    }
                }
            }
            // literal/reference: nothing to do
        },
        *node);

    if (error) {
        return std::unexpected(std::move(*error));
    }
    return node;
}

ExpressionPtrResult Parser::parsePrimary() {
    const Token& token = advance();
    switch (token.kind) {
    case TokenKind::Number:
        return std::make_unique<ExpressionTree>(Literal{.value = token.number, .span = token.span});

    case TokenKind::CellRef:
        return std::make_unique<ExpressionTree>(Ref{.name = token.lexeme, .span = token.span});

    case TokenKind::Identifier:
        if (match(TokenKind::LeftParen)) {
            std::vector<ExprPtr> args;
            if (!match(TokenKind::RightParen)) {
                do {
                    auto astResult = parseExpression(0);
                    if (!astResult) [[unlikely]] {
                        return std::unexpected(std::move(astResult).error());
                    }
                    args.push_back(std::move(astResult).value());
                } while (match(TokenKind::Comma));
                if (!match(TokenKind::RightParen)) [[unlikely]] {
                    return std::unexpected(
                        buildErrorInfo(SF_ERR_INVALID_DSL, "Missing ')' after arguments", peek().span));
                }
            }
            return std::make_unique<ExpressionTree>(
                Call{.name = token.lexeme, .args = std::move(args), .span = token.span});
        }
        return std::unexpected(
            buildErrorInfo(SF_ERR_INVALID_DSL, "Bare identifier not allowed, use <id> for cell ref", token.span));

    case TokenKind::Plus:
    case TokenKind::Minus:
    case TokenKind::Bang: {
        auto rhsResult = parseExpression(11); // unary precedence
        if (!rhsResult) [[unlikely]] {
            return std::unexpected(std::move(rhsResult).error());
        }
        return std::make_unique<ExpressionTree>(
            Unary{.op = token.kind, .rhs = std::move(rhsResult).value(), .span = token.span});
    }

    case TokenKind::LeftParen: {
        auto innerResult = parseExpression(0);
        if (!innerResult) [[unlikely]] {
            return std::unexpected(std::move(innerResult).error());
        }
        if (!match(TokenKind::RightParen)) [[unlikely]] {
            return std::unexpected(buildErrorInfo(SF_ERR_INVALID_DSL, "Expected ')'", peek().span));
        }
        return innerResult;
    }

    default:
        return std::unexpected(buildErrorInfo(SF_ERR_INVALID_DSL, "Unexpected token in expression", peek().span));
    }
}

ExpressionPtrResult Parser::parseExpression(BindingPower minBindingPower) {
    auto lhsResult = parsePrimary();
    if (!lhsResult) [[unlikely]] {
        return std::unexpected(std::move(lhsResult).error());
    }
    auto lhs = std::move(lhsResult).value();

    while (true) {
        // ternary needs special peek
        if (match(TokenKind::Question)) {
            auto thenEResult = parseExpression(0);
            if (!thenEResult) [[unlikely]] {
                return std::unexpected(std::move(thenEResult).error());
            }
            if (!match(TokenKind::Colon)) [[unlikely]] {
                return std::unexpected(buildErrorInfo(SF_ERR_INVALID_DSL, "Missing ':' in ternary", peek().span));
            }
            auto elseEResult = parseExpression(0);
            if (!elseEResult) [[unlikely]] {
                return std::unexpected(std::move(elseEResult).error());
            }
            lhs = std::make_unique<ExpressionTree>(Ternary{.cond = std::move(lhs),
                                                           .thenExpr = std::move(thenEResult).value(),
                                                           .elseExpr = std::move(elseEResult).value(),
                                                           .span = peek(-1).span});
            continue;
        }

        TokenKind nextOperator = peek().kind;
        auto lhsBindingPower = leftBindingPower(nextOperator);
        if (lhsBindingPower < minBindingPower) {
            break; // precedence finished
        }

        advance(); // consume operator
        BindingPower rhsBindingPower = rightBindingPower(nextOperator);
        auto rhsResult = parseExpression(rhsBindingPower);
        if (!rhsResult) [[unlikely]] {
            return std::unexpected(std::move(rhsResult).error());
        }

        lhs = std::make_unique<ExpressionTree>(Binary{.op = nextOperator,
                                                      .lhs = std::move(lhs),
                                                      .rhs = std::move(rhsResult).value(),
                                                      .span = peek(-1).span});
    }
    return std::move(lhs);
}

ExpressionPtrResult Parser::parse(bool fold) {
    auto astResult = parseExpression();
    if (!astResult) [[unlikely]] {
        return std::unexpected(std::move(astResult).error());
    }
    if (!match(TokenKind::EndOfFile)) [[unlikely]] {
        return std::unexpected(buildErrorInfo(SF_ERR_INVALID_DSL, "Unexpected trailing tokens", peek().span));
    }

    if (fold) [[likely]] {
        astResult = foldConstants(std::move(astResult).value());
    }

    if (astResult) {
        auto result = verify(astResult.value());
        if (!result) [[unlikely]] {
            return std::unexpected(std::move(result).error());
        }
    }

    return astResult;
}

} // namespace statforge