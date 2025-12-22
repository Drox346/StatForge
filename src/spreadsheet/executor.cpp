#include "executor.hpp"
#include "types/definitions.hpp"

#include <stack>

namespace statforge::spreadsheet {

void Executor::setEvaluationType(EvaluationType type) {
    if (type == EvaluationType::Recursive) {
        evaluateImpl = &Executor::evaluateRecursive;
    } else {
        evaluateImpl = &Executor::evaluateIterative;
    }
}

void Executor::reset() {
    _dirtyLeaves.clear();
}

void Executor::markDirty(CellId const& id) {
    std::stack<CellId> work;
    work.push(id);

    while (!work.empty()) {
        auto currentId = work.top();
        work.pop();
        auto& currentCell = _graph.cell(currentId);
        const bool hasFormula = currentCell.type != CellType::Value;

        if (currentCell.dirty) {
            continue;
        }

        currentCell.dirty = hasFormula;

        const auto& dependents = static_cast<const Graph&>(_graph).dependents(currentId);
        for (auto const& dependent : dependents) {
            work.push(dependent);
        }
        if (dependents.empty() && hasFormula) {
            _dirtyLeaves.emplace_back(currentId);
        }
    }
}

void Executor::markAsDirtyLeaf(CellId const& id) {
    _dirtyLeaves.emplace_back(id);
}

void Executor::remove(CellId const& id) {
    _dirtyLeaves.erase(std::ranges::find(_dirtyLeaves, id));
}

void Executor::evaluate(CellId const& id) {
    (this->*evaluateImpl)(id);
}

CellValueResult Executor::getCellValue(CellId const& id) {
    SF_RETURN_UNEXPECTED_IF(!_graph.contains(id),
                            SF_ERR_CELL_NOT_FOUND,
                            std::format(R"(Trying to set value of non-existing cell "{}")", id));

    auto& cell = _graph.cell(id);
    if (cell.dirty) {
        evaluate(id);
    }
    return cell.value;
}

VoidResult Executor::evaluate() {
    for (auto const& cell : _dirtyLeaves) {
        evaluate(cell);
    }
    _dirtyLeaves.clear();
    return {};
}

void Executor::evaluateRecursive(CellId const& id) {
    auto& cell = _graph.cell(id);
    if (!cell.dirty) {
        return;
    }

    for (auto const& dependency : _graph.dependencies(id)) {
        evaluateRecursive(dependency);
    }

    if (cell.type != CellType::Value) {
        cell.value = cell.formula();
    }
    cell.dirty = false;
}

void Executor::evaluateIterative(CellId const& id) {
    if (!_graph.cell(id).dirty) {
        return;
    }

    enum class VisitState : uint8_t { Unvisited, Visiting, Visited };
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

        auto& cell = _graph.cell(currentId);

        if (state == VisitState::Unvisited) {
            state = VisitState::Visiting;

            if (!cell.dirty) {
                stack.pop();
                state = VisitState::Visited;
                continue;
            }

            for (const auto& dep : static_cast<const Graph&>(_graph).dependencies(currentId)) {
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

} // namespace statforge::spreadsheet