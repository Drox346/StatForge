#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace statforge {

struct Span {
    std::size_t line{1}, column{1};
};

enum class TokenKind : uint8_t {
    // single-char operators
    EndOfFile = 0,
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

class Tokenizer {
public:
    explicit Tokenizer(std::string_view src) : _source{src} {
    }
    std::vector<Token> tokenize();

private:
    [[noreturn]] void fail(char const* message) const;

    [[nodiscard]] char peek(std::size_t off = 0) const;
    [[nodiscard]] bool match(char expected);
    void advance();
    void add(TokenKind kind);
    void number();
    void identifierOrKeyword();
    void cellReference();

    std::string_view _source;
    std::vector<Token> _tokens;
    std::size_t _pos{0};
    Span _here{};
};

} // namespace statforge