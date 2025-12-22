#pragma once

#include "error/internal/error.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace statforge::dsl {

enum class TokenKind : uint8_t {
    EndOfFile = 0,

    // single-char operators
    LeftParen,
    RightParen,
    Comma,
    Question,
    Colon,
    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    Bang,

    // two-char operators, comparisons
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    EqualEqual,
    BangEqual,
    AndAnd,
    OrOr,

    // literals, identifiers
    Number,
    Identifier,
    CellRef
};

struct Token {
    TokenKind kind{};
    std::string_view lexeme;
    Span span{};
    double number{};
};

using TokenResult = Result<std::vector<Token>>;

class Tokenizer {
public:
    explicit Tokenizer(std::string_view src) : _source{src} {
    }
    TokenResult tokenize();

private:
    [[nodiscard]] char peek(std::size_t off = 0) const;
    [[nodiscard]] bool match(char expected);
    void advance();
    void add(TokenKind kind);
    void number();
    void identifierOrKeyword();
    VoidResult cellReference();

    std::string_view _source;
    std::vector<Token> _tokens;
    std::size_t _pos{0};
    Span _here{};
};

} // namespace statforge::dsl