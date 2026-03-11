#pragma once

#include "dsl/ast.hpp"
#include "types/definitions.hpp"

#include <functional>
#include <string_view>
#include <vector>

namespace statforge::dsl {

struct Context {
    std::function<double(std::string_view)> nodeLookup;
};

double evaluate(ExpressionTree const& expression, Context const& context);

std::vector<NodeId> extractDependencies(ExpressionTree const& expression);

} // namespace statforge::dsl