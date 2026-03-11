#pragma once

#include "types/definitions.hpp"

#include <functional>

namespace statforge::statkernel {

using NodeFormula = std::function<NodeValue()>;

enum class NodeType : u_int8_t {
    Value,
    Formula,
    Aggregator,
};

struct Node {
    NodeFormula formula;
    NodeValue value{};
    NodeType type{};
    bool dirty{false};
};

} // namespace statforge::statkernel