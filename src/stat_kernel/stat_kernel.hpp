#pragma once

#include "error/internal/error.hpp"
#include "stat_kernel/compiler.hpp"
#include "stat_kernel/executor.hpp"
#include "stat_kernel/graph.hpp"

namespace statforge {

using CellValueResult = Result<CellValue>;

class StatKernel {
public:
    StatKernel();

    VoidResult createAggregatorCell(CellId const& id,
                                    std::vector<CellId> const& dependencies); //compiler/eval
    VoidResult createFormulaCell(CellId const& id, std::string_view formula); //compiler/eval
    VoidResult createValueCell(CellId const& id, double value);               //compiler/eval

    VoidResult removeCell(CellId const& id);

    VoidResult setCellValue(CellId const& id, CellValue value);
    VoidResult setCellFormula(CellId const& id, std::string_view formula);
    VoidResult setCellDependencies(CellId const& id, std::vector<CellId> const& dependencies);
    [[nodiscard]] CellValueResult getCellValue(CellId const& id);

    VoidResult evaluate();

    void reset();
    void setEvaluationType(statkernel::Executor::EvaluationType evaluationType);

private:
    statkernel::Graph _graph;
    statkernel::Compiler _compiler;
    statkernel::Executor _executor;
};

} // namespace statforge