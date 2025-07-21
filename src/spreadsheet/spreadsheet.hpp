#pragma once

#include "dsl/ast.hpp"
#include "spreadsheet/cell.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace statforge {

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

    void createAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies);
    void createFormulaCell(CellId const& id, std::string_view const& formula);
    void createValueCell(CellId const& id, double value);
    void setCellDependencies(CellId const& id, std::vector<CellId> const& dependencies);
    void removeCell(CellId const& id);

    void setCellValue(CellId const& id, CellValue value);
    CellValue getCellValue(CellId const& id);

    void evaluate(CellId const& id);
    void evaluate();

    void reset();

private:
    void setDirty(CellId const& id);

    void evaluateRecursive(CellId const& id);
    void evaluateIterative(CellId const& id);
    void (Spreadsheet::*evaluateImpl)(CellId const& id) = &Spreadsheet::evaluateIterative;

    CellFormula makeThunk(std::unique_ptr<statforge::ExpressionTree> ast);
    void wireDependencies(CellId const& id, std::unordered_set<std::string_view> const& depNames);

    static Spreadsheet::CompiledFormula compileDSL(std::string const& text);
    static inline CellId resolveCellId(std::string_view const& id);

    std::unordered_map<CellId, Cell> _cells;
    std::vector<CellId> _dirtyLeaves;

    std::unordered_map<CellId, std::vector<CellId>> _cellDependencies;
    std::unordered_map<CellId, std::vector<CellId>> _cellDependents;
};

} // namespace statforge