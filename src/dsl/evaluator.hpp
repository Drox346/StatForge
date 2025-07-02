#pragma once
#include "dsl/ast.hpp"
#include <functional>
#include <string_view>
#include <unordered_set>

namespace statforge {

struct Context {
    double const* cellValues{nullptr};
    std::function<double(std::string_view const&)> cellLookup;
};

double evaluate(ExpressionTree const& expression, Context const& context);

std::unordered_set<std::string_view> extractDependencies(ExpressionTree const& expression);

} // namespace statforge