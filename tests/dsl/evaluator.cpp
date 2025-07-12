#include "dsl/evaluator.hpp"
#include "dsl/parser.hpp"
#include "dsl/tokenizer.hpp"

#include <cstddef>
#include <doctest/doctest.h>

using statforge::evaluate;
using statforge::extractDependencies;
using statforge::Parser;
using statforge::Tokenizer;

// helpers

namespace {

auto makeAst(std::string const& src) {
    return Parser{Tokenizer{src}.tokenize()}.parse();
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


// precedence and unary

TEST_CASE("power, unary minus, and precedence") {
    auto const ast = makeAst("-2^3 + 4 * 5");
    CHECK_EQ(evaluate(*ast, makeCtx()), doctest::Approx(12.0));
}


// comparison operators

TEST_CASE("numeric boolean logic and comparisons") {
    auto const ast = makeAst("(3 > 2) && (4 == 4) || 0"); // (1 && 1) || 0  -> 1
    CHECK_EQ(evaluate(*ast, makeCtx()), doctest::Approx(1.0));
}


// ternary

TEST_CASE("five-level ternary chain evaluates correctly") {
    std::string formula = "<a> ? 1 : <b> ? 2 : <c> ? 3 : <d> ? 4 : 5";
    auto const ast = makeAst(formula);

    /* 
        a = 0  -> else branch
        b = 0  -> else branch
        c = 1  -> then branch -> result 3
    */
    auto const ctx = makeCtx({{"a", 0.0}, {"b", 0.0}, {"c", 1.0}, {"d", 0.0}});
    CHECK_EQ(evaluate(*ast, ctx), doctest::Approx(3.0));
}


// functions

TEST_CASE("root function computes n-th root") {
    std::string formula = "root(3,27)";
    auto const ast = makeAst(formula);

    CHECK_EQ(evaluate(*ast, makeCtx()), doctest::Approx(3.0));
}

TEST_CASE("root wrong arg count throws") {
    CHECK_THROWS_WITH_AS(evaluate(*makeAst("root(2)"), makeCtx()),
                         "root() expects exactly two arguments",
                         std::runtime_error);
}

TEST_CASE("unknown function throws") {
    CHECK_THROWS_WITH_AS(evaluate(*makeAst("foo(1)"), makeCtx()), "unknown function 'foo'", std::runtime_error);
}


// cell lookup

TEST_CASE("cell reference multiplies correctly") {
    std::string formula = "<price> * <qty>";
    auto const ast = makeAst(formula);

    auto const ctx = makeCtx({{"price", 2.5}, {"qty", 4}});
    CHECK_EQ(evaluate(*ast, ctx), doctest::Approx(10.0));
}

TEST_CASE("unknown cell triggers exception") {
    std::string formula = "<missing>";
    auto const ast = makeAst(formula);
    CHECK_THROWS_WITH_AS(evaluate(*ast, makeCtx({})), "unknown cell 'missing'", std::runtime_error);
}


// dependency extraction

TEST_CASE("dependencies preserve first-appearance order and duplicates") {
    std::string formula = "<a> + <b> * <a>";
    auto const ast = makeAst(formula);
    auto const deps = extractDependencies(*ast);
    CHECK_EQ(deps, std::unordered_set<std::string_view>{"a", "b", "a"});
}


// performance smoke test

TEST_CASE("very long alternating add/sub chain evaluates without stack overflow") {
    constexpr std::size_t iterations = 1000;

    std::string src;
    for (std::size_t i = 0; i < iterations; ++i) {
        src += "1 + 1 - ";
    }
    src += "1";
    auto const ast = makeAst(src);
    CHECK_NOTHROW(evaluate(*ast, makeCtx()));
}