#include "dsl/parser.hpp"
#include "dsl/error.hpp"

#include <cmath>
#include <stdexcept>

namespace statforge {

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

void Parser::expect(TokenKind kind, char const* msg) {
    if (!match(kind)) {
        throw DslError{peek().span, msg};
    }
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

ExprPtr Parser::foldConstants(ExprPtr node) {
    auto lit = [](double val, Span span) {
        return std::make_unique<ExpressionTree>(Literal{.value = val, .span = span});
    };

    std::visit(
        [&](auto& actual) {
            using Node = std::decay_t<decltype(actual)>;

            if constexpr (std::is_same_v<Node, Unary>) {
                actual.rhs = foldConstants(std::move(actual.rhs));
                if (auto const* rhs = std::get_if<Literal>(actual.rhs.get())) {
                    double val = (actual.op == TokenKind::Minus)  ? -rhs->value
                                 : (actual.op == TokenKind::Plus) ? rhs->value
                                                                  : (rhs->value == 0.0 ? 1.0 : 0.0); // Bang
                    node = lit(val, actual.span);
                }
            } else if constexpr (std::is_same_v<Node, Binary>) {
                actual.lhs = foldConstants(std::move(actual.lhs));
                actual.rhs = foldConstants(std::move(actual.rhs));

                if (auto const* lhs = std::get_if<Literal>(actual.lhs.get())) {
                    if (auto const* rhs = std::get_if<Literal>(actual.rhs.get())) {
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
                            if (rhs->value == 0.0) {
                                throw DslError{actual.span, "division by zero"};
                            }
                            val = lhs->value / rhs->value;
                            break;
                        case TokenKind::Caret:
                            val = std::pow(lhs->value, rhs->value);
                            break;
                        default:
                            return; // don’t fold logic comps
                        }
                        node = lit(val, actual.span);
                    }
                }
            } else if constexpr (std::is_same_v<Node, Ternary>) {
                actual.cond = foldConstants(std::move(actual.cond));
                actual.thenExpr = foldConstants(std::move(actual.thenExpr));
                actual.elseExpr = foldConstants(std::move(actual.elseExpr));

                if (auto const* condition = std::get_if<Literal>(actual.cond.get())) {
                    node = (condition->value != 0.0) ? std::move(actual.thenExpr) : std::move(actual.elseExpr);
                }
            } else if constexpr (std::is_same_v<Node, Call>) {
                for (auto& arg : actual.args) {
                    arg = foldConstants(std::move(arg));
                }
            }
            // literal and reference: nothing to do
        },
        *node);

    return node;
}

ExprPtr Parser::parsePrimary() {
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
                    args.push_back(parseExpression(0));
                } while (match(TokenKind::Comma));
                expect(TokenKind::RightParen, "missing ')' after arguments");
            }
            return std::make_unique<ExpressionTree>(
                Call{.name = token.lexeme, .args = std::move(args), .span = token.span});
        }
        throw std::runtime_error{"bare identifier not allowed; use <id> for cell ref"};

    case TokenKind::Plus:
    case TokenKind::Minus:
    case TokenKind::Bang: {
        auto rhs = parseExpression(11); // unary precedence
        return std::make_unique<ExpressionTree>(Unary{.op = token.kind, .rhs = std::move(rhs), .span = token.span});
    }

    case TokenKind::LeftParen: {
        auto inner = parseExpression(0);
        expect(TokenKind::RightParen, "expected ')'");
        return inner;
    }

    default:
        throw std::runtime_error{"unexpected token in expression"};
    }
}

ExprPtr Parser::parseExpression(BindingPower minBindingPower) {
    auto lhs = parsePrimary();

    while (true) {
        // ternary needs special peek
        if (match(TokenKind::Question)) {
            auto thenE = parseExpression(0);
            expect(TokenKind::Colon, "missing ':' in ternary");
            auto elseE = parseExpression(0);
            lhs = std::make_unique<ExpressionTree>(Ternary{.cond = std::move(lhs),
                                                           .thenExpr = std::move(thenE),
                                                           .elseExpr = std::move(elseE),
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
        auto rhs = parseExpression(rhsBindingPower);

        lhs = std::make_unique<ExpressionTree>(
            Binary{.op = nextOperator, .lhs = std::move(lhs), .rhs = std::move(rhs), .span = peek(-1).span});
    }
    return lhs;
}

ExprPtr Parser::parse(bool fold) {
    ExprPtr ast = parseExpression();
    expect(TokenKind::EndOfFile, "unexpected trailing tokens");

    if (fold) {
        ast = foldConstants(std::move(ast));
    }
    return ast;
}

} // namespace statforge