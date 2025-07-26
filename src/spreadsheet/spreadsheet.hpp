#pragma once

#include "common/internal/error.hpp"
//#include "debug/graph.hpp"
#include "dsl/ast.hpp"
#include "spreadsheet/cell.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace statforge {

using CellValueResult = Result<CellValue>;

class Spreadsheet {
public:
    enum class EvaluationType : uint8_t {
        Iterative,
        Recursive,
    };

    struct CompiledFormula {
        std::unique_ptr<statforge::ExpressionTree> ast;
        std::unordered_set<std::string_view> deps;
    };

    void setEvaluationType(EvaluationType type);

    VoidResult createAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies);
    VoidResult createFormulaCell(CellId const& id, std::string_view formula);
    VoidResult createValueCell(CellId const& id, double value);
    VoidResult setCellDependencies(CellId const& id,
                                   std::vector<CellId> const& dependencies,
                                   bool skipCycleCheck = false);
    VoidResult removeCell(CellId const& id);

    VoidResult setCellValue(CellId const& id, CellValue value);
    [[nodiscard]] CellValueResult getCellValue(CellId const& id);

    VoidResult evaluate();

    void reset();

private:
    void setDirty(CellId const& id);

    void evaluate(CellId const& id);
    void evaluateRecursive(CellId const& id);
    void evaluateIterative(CellId const& id);
    void (Spreadsheet::*evaluateImpl)(CellId const& id) = &Spreadsheet::evaluateIterative;

    CellFormula makeThunk(std::unique_ptr<statforge::ExpressionTree> ast);
    VoidResult wireDependencies(CellId const& id,
                                std::unordered_set<std::string_view> const& depNames,
                                bool skipCycleCheck = false);

    static inline CellId resolveCellId(std::string_view const& id);

    std::unordered_map<CellId, Cell> _cells;
    std::vector<CellId> _dirtyLeaves;
    std::unordered_map<CellId, std::vector<CellId>> _cellDependencies;
    std::unordered_map<CellId, std::vector<CellId>> _cellDependents;
};

} // namespace statforge