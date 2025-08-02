#pragma once

#include "common/internal/error.hpp"
#include "dsl/ast.hpp"
#include "dsl/tokenizer.hpp"

namespace statforge {

using ExprPtrResult = Result<ExprPtr>;

class Parser {
public:
    explicit Parser(std::vector<Token> const& tokens) : _tokens{tokens} {
    }
    [[nodiscard]] ExprPtrResult parse(bool fold = true);

private:
    using BindingPower = int;

    VoidResult verify(ExprPtr const&);

    [[nodiscard]] const Token& peek(std::size_t offset = 0) const;
    bool match(TokenKind kind);
    const Token& advance();

    ExprPtrResult parseExpression(BindingPower minBindingPower = 0);
    ExprPtrResult parsePrimary();

    static BindingPower leftBindingPower(TokenKind);
    static BindingPower rightBindingPower(TokenKind);
    static ExprPtrResult foldConstants(ExprPtr);

    std::vector<Token> const& _tokens;
    std::size_t _pos{0};
};

} // namespace statforge