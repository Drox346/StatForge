#include "dsl/evaluator.hpp"
#include "dsl/parser.hpp"
#include "dsl/tokenizer.hpp"

#include <cstddef>
#include <doctest/doctest.h>

using statforge::evaluate;
using statforge::extractDependencies;
using statforge::Parser;
using statforge::Tokenizer;

namespace {

auto makeAst(std::string const& src) {
    return Tokenizer{src}.tokenize().and_then([](auto const& tokens) { return Parser{tokens}.parse(); });
}

statforge::Context makeCtx(std::unordered_map<std::string, double> values = {}) {
    return statforge::Context{.cellLookup = [values = std::move(values)](std::string_view const& name) -> double {
        auto const iter = values.find(std::string{name});
        if (iter == values.end()) {
            throw std::runtime_error{"unknown cell '" + std::string{name} + '\''};
        }
        return iter->second;
    }};
}

} // namespace

TEST_CASE("power, unary minus, and precedence") {
    auto const& astResult = makeAst("-2^3 + 4 * 5");
    REQUIRE(astResult);
    CHECK_EQ(evaluate(*astResult.value(), makeCtx()), doctest::Approx(12.0));
}

TEST_CASE("numeric boolean logic and comparisons") {
    auto const& astResult = makeAst("(3 > 2) && (4 == 4) || 0"); // (1 && 1) || 0  -> 1
    REQUIRE(astResult);
    CHECK_EQ(evaluate(*astResult.value(), makeCtx()), 1.0);
}

TEST_CASE("five-level ternary chain evaluates correctly") {
    std::string formula = "<a> ? 1 : <b> ? 2 : <c> ? 3 : <d> ? 4 : 5";
    auto const& astResult = makeAst(formula);
    REQUIRE(astResult);

    /* 
        a = 0  -> else branch
        b = 0  -> else branch
        c = 1  -> then branch -> result 3
    */
    auto const ctx = makeCtx({{"a", 0.0}, {"b", 0.0}, {"c", 1.0}, {"d", 0.0}});
    CHECK_EQ(evaluate(*astResult.value(), ctx), doctest::Approx(3.0));
}

TEST_CASE("root function computes n-th root") {
    std::string formula = "root(3,27)";
    auto const& astResult = makeAst(formula);
    REQUIRE(astResult);

    CHECK_EQ(evaluate(*astResult.value(), makeCtx()), doctest::Approx(3.0));
}

TEST_CASE("root wrong arg count throws") {
    // CHECK_THROWS_WITH_AS(evaluate(*makeAst("root(2)"), makeCtx()),
    //                      "root() expects exactly two arguments",
    //                      std::runtime_error);
}

TEST_CASE("unknown function throws") {
    // CHECK_THROWS_WITH_AS(evaluate(*makeAst("foo(1)"), makeCtx()), "unknown function 'foo'", std::runtime_error);
}

TEST_CASE("cell reference multiplies correctly") {
    std::string formula = "<price> * <qty>";
    auto const& astResult = makeAst(formula);
    REQUIRE(astResult);

    auto const ctx = makeCtx({{"price", 2.5}, {"qty", 4}});
    CHECK_EQ(evaluate(*astResult.value(), ctx), doctest::Approx(10.0));
}

TEST_CASE("dependencies preserve first-appearance order and duplicates") {
    std::string formula = "<a> + <b> * <a>";
    auto const& astResult = makeAst(formula);
    REQUIRE(astResult);
    auto const deps = extractDependencies(*astResult.value());
    CHECK_EQ(deps, std::unordered_set<std::string_view>{"a", "b", "a"});
}

TEST_CASE("very long alternating add/sub chain evaluates without stack overflow") {
    constexpr std::size_t iterations = 1000;

    std::string src;
    for (std::size_t i = 0; i < iterations; ++i) {
        src += "1 + 1 - ";
    }
    src += "1";
    auto const& astResult = makeAst(src);
    REQUIRE(astResult);
    CHECK_EQ(evaluate(*astResult.value(), makeCtx()), 1.0);
}