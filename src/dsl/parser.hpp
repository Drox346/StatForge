#pragma once

#include "dsl/ast.hpp"
#include "dsl/tokenizer.hpp"

namespace statforge {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens) : _tokens{tokens} {
    }
    ExprPtr parse(bool fold = true);

private:
    using BindingPower = int;

    [[nodiscard]] const Token& peek(std::size_t offset = 0) const;
    bool match(TokenKind kind);
    const Token& advance();
    void expect(TokenKind kind, char const* msg);

    ExprPtr parseExpression(BindingPower minBindingPower = 0);
    ExprPtr parsePrimary();

    static BindingPower leftBindingPower(TokenKind);
    static BindingPower rightBindingPower(TokenKind);
    static ExprPtr foldConstants(ExprPtr);

    const std::vector<Token>& _tokens;
    std::size_t _pos{0};
};

} // namespace statforge