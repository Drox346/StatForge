#include "tokenizer.hpp"
#include "error/internal/error.hpp"

#include <cctype>
#include <charconv>
#include <expected>
#include <format>

namespace statforge::dsl {

namespace {

inline bool isReferenceChar(char c) {
    return (std::isalnum(c) != 0) || c == '_';
}

} // namespace

char Tokenizer::peek(std::size_t off) const {
    return _pos + off < _source.size() ? _source[_pos + off] : '\0';
}

bool Tokenizer::match(char expected) {
    if (peek() != expected) {
        return false;
    }
    advance();
    return true;
}

void Tokenizer::advance() {
    if (peek() == '\n') {
        ++_here.line;
        _here.column = 1;
    } else {
        ++_here.column;
    }
    ++_pos;
}

void Tokenizer::add(TokenKind k) {
    _tokens.push_back(Token{.kind = k, .lexeme = _source.substr(_pos - 1, 1), .span = _here});
}

void Tokenizer::number() {
    const std::size_t start = _pos;
    while (std::isdigit(peek())) {
        advance();
    }
    if (peek() == '.' && std::isdigit(peek(1))) {
        advance(); // consume '.'
        while (std::isdigit(peek())) {
            advance();
        }
    }
    const std::string_view lex = _source.substr(start, _pos - start);
    double value{};
    std::from_chars(lex.begin(), lex.end(), value);
    _tokens.push_back(Token{.kind = TokenKind::Number, .lexeme = lex, .span = _here, .number = value});
}

void Tokenizer::identifierOrKeyword() {
    const std::size_t start = _pos;
    while (isReferenceChar(peek())) {
        advance();
    }
    const std::string_view lex = _source.substr(start, _pos - start);

    if (lex == "true" || lex == "false") {
        _tokens.push_back(
            Token{.kind = TokenKind::Number, .lexeme = lex, .span = _here, .number = lex == "true" ? 1.0 : 0.0});
        return;
    }
    _tokens.push_back(Token{.kind = TokenKind::Identifier, .lexeme = lex, .span = _here});
}

VoidResult Tokenizer::cellReference() {
    Span begin = _here;
    advance(); // skip leading '<'
    const std::size_t start = _pos;
    while (isReferenceChar(peek())) {
        advance();
    }
    const auto name = _source.substr(start, _pos - start);
    SF_RETURN_UNEXPECTED_IF_SPAN(peek() != '>',
                                 SF_ERR_INVALID_DSL,
                                 std::format(R"(Unterminated cell reference)"),
                                 _here);

    advance(); // consume '>'
    _tokens.push_back(Token{.kind = TokenKind::CellRef, .lexeme = name, .span = begin});

    return {};
}

TokenResult Tokenizer::tokenize() {
    while (peek() != '\0') {
        char currentCharacter = peek();
        advance();

        if (std::isspace(currentCharacter)) {
            continue;
        }

        switch (currentCharacter) {
        case '(':
            add(TokenKind::LeftParen);
            continue;
        case ')':
            add(TokenKind::RightParen);
            continue;
        case ',':
            add(TokenKind::Comma);
            continue;
        case '?':
            add(TokenKind::Question);
            continue;
        case ':':
            add(TokenKind::Colon);
            continue;
        case '+':
            add(TokenKind::Plus);
            continue;
        case '-':
            add(TokenKind::Minus);
            continue;
        case '*':
            add(TokenKind::Star);
            continue;
        case '/':
            add(TokenKind::Slash);
            continue;
        case '^':
            add(TokenKind::Caret);
            continue;
        case '!':
            if (match('=')) {
                _tokens.push_back(Token{.kind = TokenKind::BangEqual, .lexeme = "!=", .span = _here});
            } else {
                add(TokenKind::Bang);
            }
            continue;
        case '<':
            if (match('=')) {
                _tokens.push_back(Token{.kind = TokenKind::LessEqual, .lexeme = "<=", .span = _here});
            } else if (std::isalpha(peek())) {
                _pos--;
                _here.column--;
                auto result = cellReference();
                SF_RETURN_ERROR_IF_UNEXPECTED(result);
            } else {
                add(TokenKind::Less);
            }
            continue;
        case '>':
            if (match('=')) {
                _tokens.push_back(Token{.kind = TokenKind::GreaterEqual, .lexeme = ">=", .span = _here});
            } else {
                add(TokenKind::Greater);
            }
            continue;
        case '=':
            if (match('=')) {
                _tokens.push_back(Token{.kind = TokenKind::EqualEqual, .lexeme = "==", .span = _here});
            } else {
                return std::unexpected(buildErrorInfo(SF_ERR_INVALID_DSL,
                                                      std::format(R"('=' is invalid in formulas (did you mean '=='?))"),
                                                      _here));
            }
            continue;
        case '&':
            if (match('&')) {
                _tokens.push_back(Token{.kind = TokenKind::AndAnd, .lexeme = "&&", .span = _here});
            } else {
                return std::unexpected(
                    buildErrorInfo(SF_ERR_INVALID_DSL, std::format(R"(Single '&' not supported)"), _here));
            }
            continue;
        case '|':
            if (match('|')) {
                _tokens.push_back(Token{.kind = TokenKind::OrOr, .lexeme = "||", .span = _here});
            } else {
                return std::unexpected(
                    buildErrorInfo(SF_ERR_INVALID_DSL, std::format(R"(Single '|' not supported)"), _here));
            }
            continue;
        case '"':
            return std::unexpected(
                buildErrorInfo(SF_ERR_INVALID_DSL, std::format(R"(String literals not supported)"), _here));
        default:
            if (std::isdigit(currentCharacter)) {
                _pos--;
                _here.column--;
                number();
            } else if (std::isalpha(currentCharacter) || currentCharacter == '_') {
                _pos--;
                _here.column--;
                identifierOrKeyword();
            } else {
                return std::unexpected(
                    buildErrorInfo(SF_ERR_INVALID_DSL, std::format(R"(Unknown character in formula)"), _here));
            }
        }
    }
    _tokens.push_back(Token{.kind = TokenKind::EndOfFile, .lexeme = {}, .span = _here});
    return _tokens;
}

} // namespace statforge::dsl