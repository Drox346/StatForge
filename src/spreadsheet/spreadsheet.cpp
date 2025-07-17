#include "spreadsheet.hpp"
#include "common/definitions.hpp"
#include "dsl/ast.hpp"
#include "dsl/evaluator.hpp"
#include "dsl/parser.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stack>
#include <string_view>

namespace statforge {

void Spreadsheet::setEvaluationType(EvaluationType type) {
    if (type == EvaluationType::Recursive) {
        evaluateImpl = &Spreadsheet::evaluateRecursive;
    } else {
        evaluateImpl = &Spreadsheet::evaluateIterative;
    }
}

void Spreadsheet::createAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies) {
    auto aggregate = [this, id]() -> CellValue {
        CellValue value{0};

        auto& dependencies = _cellDependencies[id];
        for (auto const& dependency : dependencies) {
            value += _cells[dependency].value;
        }

        return value;
    };

    _cells[id] = {.formula = aggregate, .dirty = true, .value = 0};
    _dirtyLeaves.emplace_back(id);
    setCellDependencies(id, dependencies);
}

void Spreadsheet::createFormulaCell(CellId const& id, std::string_view const& formula) {
    auto src = std::make_shared<std::string>(formula);

    auto ast = statforge::Parser{statforge::Tokenizer{*src}.tokenize()}.parse();
    auto deps = statforge::extractDependencies(*ast);

    CellFormula thunk = [this,
                         src, // keeps string alive
                         expr = std::shared_ptr<statforge::ExpressionTree>(std::move(ast))]() -> CellValue {
        statforge::Context ctx{
            .cellLookup = [this](std::string_view const n) -> double { return _cells.at(resolveCellId(n)).value; }};
        return statforge::evaluate(*expr, ctx);
    };

    _cells[id] = {.formula = std::move(thunk), .dirty = true, .value = 0.0};
    wireDependencies(id, deps);
    _dirtyLeaves.emplace_back(id);
}

void Spreadsheet::createValueCell(CellId const& id, double value) {
    _cells[id] = {.formula = nullptr, .dirty = false, .value = value};
}

void Spreadsheet::setCellDependencies(CellId const& id, std::vector<CellId> const& dependencies) {
    _cellDependencies[id] = dependencies;
    for (const auto& dependent : dependencies) {
        _cellDependents[dependent].emplace_back(id);
    }
    setDirty(id);
}

void Spreadsheet::removeCell(CellId const& id) {
    //_leaveCells.erase(id) //FIXME
    _cells.erase(id);
    //_dirtyLeaves.erase();
    //_cellDependencies.erase() + dependent from dependencies
    //_cellDependents.erase() + dependency from dependents
}

void Spreadsheet::setCellValue(CellId const& id, CellValue value) {
    auto const& cell = _cells.find(id);
    if (cell != _cells.end()) {
        cell->second.value = value;
        setDirty(id);
    } else {
        std::cerr << "setCellValue: unknown cell ID: " << id << "\n";
        assert(false && "setCellValue: unknown cell ID");
    }
}

CellValue Spreadsheet::getCellValue(CellId const& id) {
    CellValue value{};

    auto const& cellEntry = _cells.find(id);
    if (cellEntry != _cells.end()) {
        const auto& cell = cellEntry->second;

        if (cell.dirty) {
            evaluate(id);
        }
        value = cell.value;
    } else {
        std::cerr << "getCellValue: unknown cell ID: " << id << "\n";
        assert(false && "getCellValue: unknown cell ID");
    }

    return value;
}

void Spreadsheet::evaluate(CellId const& id) {
    (this->*evaluateImpl)(id);
}

void Spreadsheet::evaluate() {
    for (auto const& cell : _dirtyLeaves) {
        evaluate(cell);
    }
    _dirtyLeaves.clear();
}

void Spreadsheet::reset() {
    _cellDependencies.clear();
    _cellDependents.clear();
    _dirtyLeaves.clear();
    _cells.clear();
}


void Spreadsheet::setDirty(CellId const& id) {
    std::stack<CellId> work;
    work.push(id);

    while (!work.empty()) {
        auto currentId = work.top();
        work.pop();
        auto& currentCell = _cells[currentId];

        if (currentCell.dirty) {
            continue;
        }

        currentCell.dirty = true;

        const auto& dependents = _cellDependents[currentId];
        for (auto const& dependent : dependents) {
            work.push(dependent);
        }
        if (dependents.empty()) {
            _dirtyLeaves.emplace_back(currentId);
        }
    }
}

void Spreadsheet::evaluateRecursive(CellId const& id) {
    auto& cell = _cells[id];
    if (cell.dirty) {
        for (auto const& dependency : _cellDependencies[id]) {
            evaluateRecursive(dependency);
        }
        if (cell.formula) {
            cell.value = cell.formula();
        }
        cell.dirty = false;
    }
}

void Spreadsheet::evaluateIterative(CellId const& id) {
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

        auto& cell = _cells[currentId];

        if (state == VisitState::Unvisited) {
            state = VisitState::Visiting;

            if (!cell.dirty) {
                stack.pop();
                state = VisitState::Visited;
                continue;
            }

            for (const auto& dep : _cellDependencies[currentId]) {
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

Spreadsheet::CompiledFormula Spreadsheet::compileDSL(std::string const& text) {
    auto ast = statforge::Parser{statforge::Tokenizer{text}.tokenize()}.parse();
    auto deps = statforge::extractDependencies(*ast);
    return {.ast = std::move(ast), .deps = std::move(deps)};
}

CellFormula Spreadsheet::makeThunk(std::unique_ptr<statforge::ExpressionTree> ast) {
    auto expressionTree = std::shared_ptr<ExpressionTree>(ast.get());

    return [this, expressionTree]() -> CellValue {
        statforge::Context const ctx{.cellLookup = [this](std::string_view const name) -> double {
            return _cells.at(resolveCellId(name)).value;
        }};

        return statforge::evaluate(*expressionTree, ctx);
    };
}

void Spreadsheet::wireDependencies(CellId const& id, std::unordered_set<std::string_view> const& depNames) {
    std::vector<CellId> deps;
    deps.reserve(depNames.size());

    for (std::string_view name : depNames) {
        deps.push_back(resolveCellId(name));
    }

    setCellDependencies(id, deps);
}

inline CellId Spreadsheet::resolveCellId(std::string_view const& id) {
    return std::string{id};
}

} // namespace statforge