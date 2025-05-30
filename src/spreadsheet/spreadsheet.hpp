#pragma once

#include "common/definitions.hpp"
#include "spreadsheet/cell.hpp"

#include <unordered_map>

namespace statforge {

class Spreadsheet {
public:
    enum class EvaluationType {
        Iterative,
        Recursive,
    };
    Spreadsheet();

    void setEvaluationType(EvaluationType type = EvaluationType::Iterative);

    void createAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies);
    void createFormulaCell(CellId const& id, std::string const& formula);
    void createValueCell(CellId const& id, double value);
    void setCellDependencies(CellId const& id, std::vector<CellId> const& dependencies);
    void removeCell(CellId const& id);

    void evaluate(CellId const& id);
    void evaluate();
    
    void reset();
    
private:
    void setDirty(CellId const& id);

    void evaluateRecursive(CellId const& id);
    void evaluateIterative(CellId const& id);
    void (Spreadsheet::*evaluateImpl)(CellId const& id) = nullptr;

    std::unordered_map<CellId, Cell> m_cells;
    std::vector<CellId> m_dirtyLeaves;

    std::unordered_map<CellId, std::vector<CellId>> m_cellDependencies;
    std::unordered_map<CellId, std::vector<CellId>> m_cellDependents;
};
    
} // namespace statforge