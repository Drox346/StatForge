#pragma once

#include "error/internal/error.hpp"
#include "stat_kernel/compiler.hpp"
#include "stat_kernel/executor.hpp"
#include "stat_kernel/graph.hpp"
#include "types/collection_operation.h"

namespace statforge {

using NodeValueResult = Result<NodeValue>;

class StatKernel {
public:
    StatKernel();

    VoidResult createCollectionNode(NodeId const& id,
                                    std::vector<NodeId> const& dependencies,
                                    SF_CollectionOperation operation = SF_COLLECTION_OP_SUM);
    VoidResult createFormulaNode(NodeId const& id, std::string_view formula);
    VoidResult createValueNode(NodeId const& id, double value);

    VoidResult removeNode(NodeId const& id);

    VoidResult setNodeValue(NodeId const& id, NodeValue value);
    VoidResult setNodeFormula(NodeId const& id, std::string_view formula);
    VoidResult setNodeDependencies(NodeId const& id, std::vector<NodeId> const& dependencies);
    [[nodiscard]] NodeValueResult getNodeValue(NodeId const& id);

    VoidResult evaluate();

    void reset();
    void setEvaluationType(statkernel::Executor::EvaluationType evaluationType);

private:
    statkernel::Graph _graph;
    statkernel::Compiler _compiler;
    statkernel::Executor _executor;
};

} // namespace statforge
