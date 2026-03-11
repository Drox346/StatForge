#pragma once

#include "stat_kernel/graph.hpp"
#include "types/definitions.hpp"

namespace statforge::statkernel {

using NodeValueResult = Result<NodeValue>;

class Executor {
public:
    Executor() = delete;
    explicit Executor(statkernel::Graph& graph) : _graph(graph) {
    }

    enum class EvaluationType : uint8_t {
        Iterative,
        Recursive,
    };
    void setEvaluationType(EvaluationType type);
    void reset();

    void markDirty(NodeId const& id);
    void markAsDirtyLeaf(NodeId const& id);
    void remove(NodeId const& id);
    [[nodiscard]] NodeValueResult getNodeValue(NodeId const& id);
    VoidResult evaluate();

private:
    void evaluate(NodeId const& id);
    void evaluateRecursive(NodeId const& id);
    void evaluateIterative(NodeId const& id);
    void (Executor::*evaluateImpl)(NodeId const& id) = &Executor::evaluateIterative;

    std::vector<NodeId> _dirtyLeaves;
    [[maybe_unused]] statkernel::Graph& _graph;
};

} // namespace statforge::statkernel