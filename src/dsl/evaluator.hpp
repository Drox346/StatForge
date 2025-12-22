#pragma once

#include "dsl/ast.hpp"
#include "types/definitions.hpp"

#include <functional>
#include <string_view>
#include <unordered_set>

namespace statforge::dsl {

struct Context {
    std::function<double(std::string_view)> cellLookup;
};

double evaluate(ExpressionTree const& expression, Context const& context);

std::unordered_set<std::string_view> extractDependencies(ExpressionTree const& expression);

} // namespace statforge::dsl