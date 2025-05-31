#include <stack>

#include "dsl/parser.hpp"
#include "spreadsheet.hpp"
#include "common/definitions.hpp"

namespace statforge {

/************************ Public ************************/

Spreadsheet::Spreadsheet() : evaluateImpl(&Spreadsheet::evaluateIterative) {}

void Spreadsheet::setEvaluationType(EvaluationType type) {
    if(type == EvaluationType::Recursive) {
        evaluateImpl = &Spreadsheet::evaluateRecursive;
    }
    else {
        evaluateImpl = &Spreadsheet::evaluateIterative;
    }
}

void Spreadsheet::createAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies) {
    auto aggregate = [this, id]()->CellValue {
        CellValue value{0};

        auto& dependencies = m_cellDependencies[id];
        for(auto const& dependency : dependencies) {
            value += m_cells[dependency].value;
        }

        return value;
    };
    
    m_cells[id] = {aggregate, true, 0};
    m_dirtyLeaves.emplace_back(id);
    setCellDependencies(id, dependencies);
}

void Spreadsheet::createFormulaCell(CellId const& id, std::string const& formula) {
    m_cells[id] = {Parser::parseFormula(formula), true, 0};
    m_dirtyLeaves.emplace_back(id);
    setCellDependencies(id, Parser::parseFormulaDependencies(formula));
}

void Spreadsheet::createValueCell(CellId const& id, double value) {
    m_cells[id] = {nullptr, false, value};
}

void Spreadsheet::setCellDependencies(CellId const& id, std::vector<CellId> const& dependencies) {
    m_cellDependencies[id] = dependencies;
    for(auto& dependent : dependencies) {
        m_cellDependents[dependent].emplace_back(id);
    }
    setDirty(id);
}

void Spreadsheet::removeCell(CellId const& id) {
    //m_leaveCells.erase(id) //FIXME
    m_cells.erase(id);
    //m_dirtyLeaves.erase();
    //m_cellDependencies.erase() + dependent from dependencies
    //m_cellDependents.erase() + dependency from dependents
}

void Spreadsheet::setCellValue(CellId const& id, CellValue value) {
    auto const& cell = m_cells.find(id);
    if(cell != m_cells.end()) {
        cell->second.value = value;
        setDirty(id);
    }
    else {
        //TODO debug
    }
}

CellValue Spreadsheet::getCellValue(CellId const& id) {
    CellValue value;

    auto const& cell = m_cells.find(id);
    if(cell != m_cells.end()) {
        value = cell->second.value;
    }
    else {
        //TODO debug
    }

    return value;
}

void Spreadsheet::evaluate(CellId const& id) {
    (this->*evaluateImpl)(id);
}

void Spreadsheet::evaluate() {
    for(auto const& cell : m_dirtyLeaves) {
        evaluate(cell);
    }
    m_dirtyLeaves.clear();
}

void Spreadsheet::reset() {
    m_cellDependencies.clear();
    m_cellDependents.clear();
    m_dirtyLeaves.clear();
    m_cells.clear();
}

/************************ Private ************************/

void Spreadsheet::setDirty(CellId const& id) {
    std::stack<CellId> work;
    work.push(id);

    while (!work.empty()) {
        auto currentId = work.top();
        work.pop();
        auto& currentCell = m_cells[currentId];

        if (currentCell.dirty) {
            continue;
        }

        currentCell.dirty = true;

        for (auto const& dependent : m_cellDependents[currentId]) {
            work.push(dependent);
        }
    }
}

void Spreadsheet::evaluateRecursive(CellId const& id) {
    auto& cell = m_cells[id];
    if(cell.dirty) {
        for(auto const& dependency : m_cellDependencies[id]) {
            evaluateRecursive(dependency);
        }
        if(cell.formula) {
            cell.value = cell.formula();
        }
        cell.dirty = false;
    }
}

void Spreadsheet::evaluateIterative(CellId const& id) {
    enum class VisitState { Unvisited, Visiting, Visited };
    std::unordered_map<CellId, VisitState> visitState;

    std::stack<CellId> stack;
    stack.push(id);

    while (!stack.empty()) {
        auto currentId = stack.top();
        auto& state = visitState[currentId];

        if (state == VisitState::Visited) {
            stack.pop();
            continue;
        }

        auto& cell = m_cells[currentId];

        if (state == VisitState::Unvisited) {
            state = VisitState::Visiting;

            if (!cell.dirty) {
                stack.pop();
                state = VisitState::Visited;
                continue;
            }

            for (const auto& dep : m_cellDependencies[currentId]) {
                if (visitState[dep] != VisitState::Visited) {
                    stack.push(dep);
                }
            }
        } else if (state == VisitState::Visiting) {
            if (cell.formula) {
                cell.value = cell.formula();
            }
            cell.dirty = false;
            state = VisitState::Visited;
            stack.pop();
        }
    }
}

} // namespace statforge