#pragma once

#include "types/definitions.hpp"
#include "types/collection_operation.h"

#include <functional>

namespace statforge::statkernel {

using NodeFormula = std::function<NodeValue()>;

enum class NodeType : u_int8_t {
    Value,
    Formula,
    Collection,
};

struct Node {
    NodeFormula formula;
    NodeValue value{};
    NodeType type{};
    SF_CollectionOperation collectionOperation{SF_COLLECTION_OP_SUM};
    bool dirty{false};
};

} // namespace statforge::statkernel
