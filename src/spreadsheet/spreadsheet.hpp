#pragma once

#include "error/internal/error.hpp"
#include "spreadsheet/compiler.hpp"
#include "spreadsheet/executor.hpp"
#include "spreadsheet/graph.hpp"

namespace statforge {

using CellValueResult = Result<CellValue>;

class Spreadsheet {
public:
    Spreadsheet();

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
    void setEvaluationType(spreadsheet::Executor::EvaluationType evaluationType);

private:
    spreadsheet::Graph _graph;
    spreadsheet::Compiler _compiler;
    spreadsheet::Executor _executor;
};

} // namespace statforge