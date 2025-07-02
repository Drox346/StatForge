#include "dsl/ast.hpp"

#include <sstream>

namespace statforge {

namespace {

std::string opName(TokenKind kind) {
    switch (kind) {
    case TokenKind::Plus:
        return "+";
    case TokenKind::Minus:
        return "-";
    case TokenKind::Star:
        return "*";
    case TokenKind::Slash:
        return "/";
    case TokenKind::Caret:
        return "^";
    case TokenKind::Less:
        return "<";
    case TokenKind::LessEqual:
        return "<=";
    case TokenKind::Greater:
        return ">";
    case TokenKind::GreaterEqual:
        return ">=";
    case TokenKind::EqualEqual:
        return "==";
    case TokenKind::BangEqual:
        return "!=";
    case TokenKind::AndAnd:
        return "&&";
    case TokenKind::OrOr:
        return "||";
    case TokenKind::Bang:
        return "!";
    default:
        return "?";
    }
}
void dump(const ExpressionTree& expression, std::ostringstream& out) {
    std::visit(
        [&](auto&& node) {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Literal>) {
                out << node.value;
            } else if constexpr (std::is_same_v<T, Ref>) {
                out << '<' << node.name << '>';
            } else if constexpr (std::is_same_v<T, Unary>) {
                out << '(' << opName(node.op) << ' ';
                dump(*node.rhs, out);
                out << ')';
            } else if constexpr (std::is_same_v<T, Binary>) {
                out << '(' << opName(node.op) << ' ';
                dump(*node.lhs, out);
                out << ' ';
                dump(*node.rhs, out);
                out << ')';
            } else if constexpr (std::is_same_v<T, Ternary>) {
                out << "(? ";
                dump(*node.cond, out);
                out << ' ';
                dump(*node.thenExpr, out);
                out << ' ';
                dump(*node.elseExpr, out);
                out << ')';
            } else if constexpr (std::is_same_v<T, Call>) {
                out << "(call " << node.name;
                for (auto& arg : node.args) {
                    out << ' ';
                    dump(*arg, out);
                }
                out << ')';
            }
        },
        expression);
}

} // namespace

std::string dumpSExpr(const ExpressionTree& expression) {
    std::ostringstream out;
    dump(expression, out);
    return out.str();
}

} // namespace statforge