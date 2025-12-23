#pragma once

#include "stat_kernel/graph.hpp"
#include "types/definitions.hpp"

namespace statforge::statkernel {

using CellValueResult = Result<CellValue>;

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

    void markDirty(CellId const& id);
    void markAsDirtyLeaf(CellId const& id);
    void remove(CellId const& id);
    [[nodiscard]] CellValueResult getCellValue(CellId const& id);
    VoidResult evaluate();

private:
    void evaluate(CellId const& id);
    void evaluateRecursive(CellId const& id);
    void evaluateIterative(CellId const& id);
    void (Executor::*evaluateImpl)(CellId const& id) = &Executor::evaluateIterative;

    std::vector<CellId> _dirtyLeaves;
    [[maybe_unused]] statkernel::Graph& _graph;
};

} // namespace statforge::statkernel