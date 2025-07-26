#pragma once

#include "common/internal/error.hpp"
#include "dsl/ast.hpp"
#include "dsl/tokenizer.hpp"

namespace statforge {

using ExpressionPtrResult = Result<ExprPtr>;

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens) : _tokens{tokens} {
    }
    [[nodiscard]] ExpressionPtrResult parse(bool fold = true);

private:
    using BindingPower = int;

    VoidResult verify(ExprPtr const&);

    [[nodiscard]] const Token& peek(std::size_t offset = 0) const;
    bool match(TokenKind kind);
    const Token& advance();

    ExpressionPtrResult parseExpression(BindingPower minBindingPower = 0);
    ExpressionPtrResult parsePrimary();

    static BindingPower leftBindingPower(TokenKind);
    static BindingPower rightBindingPower(TokenKind);
    static ExpressionPtrResult foldConstants(ExprPtr);

    std::vector<Token> const& _tokens;
    std::size_t _pos{0};
};

} // namespace statforge