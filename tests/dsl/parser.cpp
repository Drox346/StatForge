#include "dsl/parser.hpp"
#include "dsl/ast.hpp"
#include "dsl/tokenizer.hpp"
#include "error/error.h"

#include <doctest/doctest.h>

using statforge::dsl::dumpSExpr;
using statforge::dsl::Parser;
using statforge::dsl::Tokenizer;

// helpers

namespace {

static std::string sexpr(std::string const& src) {
    auto tokenResult = Tokenizer{src}.tokenize();
    REQUIRE(tokenResult);
    auto astResult = Parser{tokenResult.value()}.parse(false); //no constant folding
    REQUIRE(astResult);
    return dumpSExpr(*astResult.value());
}

statforge::ErrorInfo parseError(std::string const& src) {
    auto tokenResult = Tokenizer{src}.tokenize();
    REQUIRE(tokenResult);
    auto astResult = Parser{tokenResult.value()}.parse(false);
    REQUIRE_FALSE(astResult);
    return astResult.error();
}

} // namespace

// operator precedence

TEST_CASE("addition binds looser than multiplication") {
    CHECK(sexpr("1 + 2 * 3") == "(+ 1 (* 2 3))");
    CHECK(sexpr("1 * 2 + 3") == "(+ (* 1 2) 3)");
}

TEST_CASE("power is tighter than unary - but right-associative") {
    CHECK(sexpr("-2^3") == "(- (^ 2 3))");
    CHECK(sexpr("2 ^ 3 ^ 2") == "(^ 2 (^ 3 2))");
}

TEST_CASE("logical && binds tighter than ||") {
    CHECK(sexpr("1 || 0 && 0") == "(|| 1 (&& 0 0))");
}


// unary chains and parentheses

TEST_CASE("multiple unary operators chain correctly") {
    CHECK(sexpr("!- -3") == "(! (- (- 3)))");
}

TEST_CASE("parentheses override precedence") {
    CHECK(sexpr("(1 + 2) * 3") == "(* (+ 1 2) 3)");
}


// ternary associativity

TEST_CASE("nested ternary is right-associative") {
    std::string const expected = "(? <a> (? <b> 1 2) 3)";
    CHECK(sexpr("<a> ? <b> ? 1 : 2 : 3") == expected);
}

TEST_CASE("ternary inside binary still respects precedence") {
    std::string const expected = "(+ 1 (? 0 2 3))";
    CHECK(sexpr("1 + (0 ? 2 : 3)") == expected);
}


// function calls and argument lists

TEST_CASE("nested call as argument") {
    CHECK(sexpr("root(2, root(3,8))") == "(call root 2 (call root 3 8))");
}

TEST_CASE("long argument list") {
    CHECK_NOTHROW(sexpr("root(1,2,3,4,5,6,7,8,9)"));
}


// node reference variants

TEST_CASE("node ref may include underscores and digits") {
    CHECK(sexpr("<some_weird_123_name>") == "<some_weird_123_name>");
}


// boolean

TEST_CASE("boolean keywords lower to numeric 0 / 1") {
    CHECK(sexpr("true && false") == "(&& 1 0)");
}

// error handling

TEST_CASE("missing right parenthesis reports invalid dsl") {
    auto error = parseError("(1 + 2");
    CHECK_EQ(error.errorCode, SF_ERR_INVALID_DSL);
    CHECK_EQ(error.message, "Expected ')'");
}

TEST_CASE("dangling operator at end of expression reports invalid dsl") {
    auto error = parseError("1 + ");
    CHECK_EQ(error.errorCode, SF_ERR_INVALID_DSL);
    CHECK_EQ(error.message, "Unexpected token in expression");
}

TEST_CASE("bare identifier is rejected") {
    auto error = parseError("foo");
    CHECK_EQ(error.errorCode, SF_ERR_INVALID_DSL);
    CHECK_EQ(error.message, "Bare identifier not allowed, use <id> for node ref");
}

TEST_CASE("trailing tokens are rejected") {
    auto error = parseError("(1 + 2) 3");
    CHECK_EQ(error.errorCode, SF_ERR_INVALID_DSL);
    CHECK_EQ(error.message, "Unexpected trailing tokens");
}


// performance smoke test

TEST_CASE("long alternating add-sub chain parses") {
    constexpr std::size_t iterations = 500;

    std::string src;
    for (std::size_t i = 0; i < iterations; ++i) {
        src += "1 + 1 - ";
    }
    src += "1";

    CHECK_NOTHROW(sexpr(src));
}
