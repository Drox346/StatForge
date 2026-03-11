#include "dsl/tokenizer.hpp"
#include "error/error.h"

#include <doctest/doctest.h>

using statforge::dsl::Tokenizer;
using statforge::dsl::TokenKind;

namespace {

auto getTokenKinds(std::string_view src) {
    std::vector<TokenKind> retVal;
    auto tokenResult = Tokenizer{src}.tokenize();
    REQUIRE(tokenResult);
    for (auto& token : tokenResult.value()) {
        retVal.push_back(token.kind);
    }
    return retVal;
}

auto tokenizeError(std::string_view src) {
    auto tokenResult = Tokenizer{src}.tokenize();
    REQUIRE_FALSE(tokenResult);
    return tokenResult.error();
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

TEST_CASE("node reference and comparison") {
    auto tokens = getTokenKinds("<level> == 47");
    CHECK_EQ(tokens[0], TokenKind::NodeRef);
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

TEST_CASE("single equals reports invalid dsl") {
    auto error = tokenizeError("1 = 2");
    CHECK_EQ(error.errorCode, SF_ERR_INVALID_DSL);
    CHECK_EQ(error.message, R"('=' is invalid in formulas (did you mean '=='?))");
}

TEST_CASE("unknown character reports invalid dsl") {
    auto error = tokenizeError("1 $ 2");
    CHECK_EQ(error.errorCode, SF_ERR_INVALID_DSL);
    CHECK_EQ(error.message, "Unknown character in formula");
}
