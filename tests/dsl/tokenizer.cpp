#include "dsl/tokenizer.hpp"

#include <doctest/doctest.h>

using statforge::Tokenizer;
using statforge::TokenKind;

namespace {

auto getTokenKinds(std::string_view src) {
    std::vector<TokenKind> retVal;
    for (auto& token : Tokenizer{src}.tokenize()) {
        retVal.push_back(token.kind);
    }
    return retVal;
}

} // namespace

TEST_CASE("end of file") {
    auto tokens = getTokenKinds("1");
    CHECK_EQ(tokens, std::vector<TokenKind>{TokenKind::Number, TokenKind::EndOfFile});
}

TEST_CASE("simple arithmetic") {
    auto tokens = getTokenKinds("1 + 2 * 3");
    CHECK_EQ(tokens[0], TokenKind::Number);
    CHECK_EQ(tokens[1], TokenKind::Plus);
    CHECK_EQ(tokens[2], TokenKind::Number);
    CHECK_EQ(tokens[3], TokenKind::Star);
    CHECK_EQ(tokens[4], TokenKind::Number);
}

TEST_CASE("cell reference and comparison") {
    auto tokens = getTokenKinds("<level> == 47");
    CHECK_EQ(tokens[0], TokenKind::CellRef);
    CHECK_EQ(tokens[1], TokenKind::EqualEqual);
    CHECK_EQ(tokens[2], TokenKind::Number);
}

TEST_CASE("ternary chain") {
    auto tokens = getTokenKinds("<x> ? 1 : 2");
    CHECK_EQ(tokens[1], TokenKind::Question);
    CHECK_EQ(tokens[3], TokenKind::Colon);
}

TEST_CASE("function call") {
    auto tokens = getTokenKinds("root(3,8)");
    REQUIRE(tokens.size() >= 6);
    CHECK_EQ(tokens[0], TokenKind::Identifier);
    CHECK_EQ(tokens[1], TokenKind::LeftParen);
}

TEST_CASE("boolean keywords become numeric") {
    auto tokens = getTokenKinds("true && false");
    REQUIRE(tokens.size() == 4);
    CHECK_EQ(tokens[0], TokenKind::Number);
    CHECK_EQ(tokens[1], TokenKind::AndAnd);
    CHECK_EQ(tokens[2], TokenKind::Number);
}