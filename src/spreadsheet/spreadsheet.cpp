#include "spreadsheet.hpp"
#include "common/error.h"
#include "common/internal/definitions.hpp"
#include "common/internal/error.hpp"
#include "dsl/ast.hpp"
#include "dsl/evaluator.hpp"
#include "dsl/parser.hpp"
#include "dsl/tokenizer.hpp"
#include "spreadsheet/cell.hpp"

#include <cassert>
#include <cstdint>
#include <expected>
#include <format>
#include <memory>
#include <stack>
#include <string_view>

namespace statforge {

namespace {

bool hasPath(CellId const& src,
             CellId const& target,
             std::unordered_map<CellId, std::vector<CellId>> const& dependencyMap) {
    std::stack<CellId> work;
    std::unordered_set<CellId> visited;
    work.push(src);

    while (!work.empty()) {
        auto current = work.top();
        work.pop();
        if (current == target) {
            return true;
        }
        if (!visited.insert(current).second) {
            continue;
        }

        auto it = dependencyMap.find(current);
        if (it == dependencyMap.end()) {
            continue;
        }
        for (auto const& dep : it->second) {
            work.push(dep);
        }
    }
    return false;
}

} // namespace

void Spreadsheet::setEvaluationType(EvaluationType type) {
    if (type == EvaluationType::Recursive) {
        evaluateImpl = &Spreadsheet::evaluateRecursive;
    } else {
        evaluateImpl = &Spreadsheet::evaluateIterative;
    }
}

VoidResult Spreadsheet::createAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies) {
    if (_cells.contains(id)) [[unlikely]] {
        return std::unexpected(
            buildErrorInfo(SF_ERR_CELL_ALREADY_EXISTS, std::format(R"(Cell "{}" already exist)", id)));
    }

    static constexpr bool skipCycleCheck = true;
    auto result = setCellDependencies(id, dependencies, skipCycleCheck);
    if (!result) {
        return result;
    }

    auto aggregate = [this, id]() -> CellValue {
        CellValue value{0};

        auto& dependencies = _cellDependencies[id];
        for (auto const& dependency : dependencies) {
            value += _cells.at(dependency).value;
        }

        return value;
    };

    _cells[id] = {.formula = aggregate, .value = 0, .type = CellType::Aggregator, .dirty = true};
    _dirtyLeaves.emplace_back(id);

    return {};
}

VoidResult Spreadsheet::createFormulaCell(CellId const& id, std::string_view formula) {
    if (_cells.contains(id)) [[unlikely]] {
        return std::unexpected(
            buildErrorInfo(SF_ERR_CELL_ALREADY_EXISTS, std::format(R"(Cell "{}" already exists)", id)));
    }

    auto src = std::make_shared<std::string>(formula);

    auto astResult = Tokenizer{*src}
                         .tokenize()
                         .and_then([](auto const& tokens) { return Parser{tokens}.parse(); })
                         .transform_error([&id](auto&& error) {
                             error.message.insert(0, std::format(R"(Cell "{}": )", id));
                             return std::move(error);
                         });
    if (!astResult) [[unlikely]] {
        return std::unexpected(std::move(astResult).error());
    }

    static constexpr bool skipCycleCheck = true;
    auto result = wireDependencies(id, statforge::extractDependencies(*astResult.value()), skipCycleCheck);
    if (!result) {
        return result;
    }

    CellFormula thunk = [this,
                         src, // keeps string alive
                         expr =
                             std::shared_ptr<statforge::ExpressionTree>(std::move(astResult.value()))]() -> CellValue {
        statforge::Context ctx{
            .cellLookup = [this](std::string_view n) -> double { return _cells.at(resolveCellId(n)).value; }};
        return statforge::evaluate(*expr, ctx);
    };

    _cells[id] = {.formula = std::move(thunk), .value = 0.0, .type = CellType::Formula, .dirty = true};
    _dirtyLeaves.emplace_back(id);
    return {};
}

VoidResult Spreadsheet::createValueCell(CellId const& id, double value) {
    if (_cells.contains(id)) [[unlikely]] {
        return std::unexpected(
            buildErrorInfo(SF_ERR_CELL_ALREADY_EXISTS, std::format(R"(Cell "{}" already exists)", id)));
    }

    _cells[id] = {.formula = nullptr, .value = value, .type = CellType::Value, .dirty = false};
    return {};
}

VoidResult Spreadsheet::setCellDependencies(CellId const& id,
                                            std::vector<CellId> const& dependencies,
                                            bool skipCycleCheck) {
    for (auto const& dependency : dependencies) {
        if (!_cells.contains(dependency)) [[unlikely]] {
            return std::unexpected(
                buildErrorInfo(SF_ERR_DEPENDENCY_DOESNT_EXIST,
                               std::format(R"(Trying to add non-existing dependency "{}" to "{}")", dependency, id)));
        }
        if (dependency == id) [[unlikely]] {
            return std::unexpected(buildErrorInfo(SF_ERR_SELF_REFERENCE,
                                                  std::format(R"("{}" is trying to set itself as dependency)", id)));
        }
        if (!skipCycleCheck && hasPath(dependency, id, _cellDependencies)) [[unlikely]] {
            return std::unexpected(
                buildErrorInfo(SF_ERR_DEPENDENCY_LOOP,
                               std::format(R"(Trying to set dependency of "{}" with cyclic dependency)", id)));
        }
    }

    const auto previousDependencies = _cellDependencies[id];

    //FIXME rework to undo existing dependencies

    _cellDependencies[id] = dependencies;
    for (const auto& dependent : dependencies) {
        if (!_cells.contains(dependent)) [[unlikely]] {
            if (previousDependencies.empty()) {
                _cellDependencies.erase(id);
            } else {
                _cellDependencies[id] = previousDependencies;
            }

            return std::unexpected(
                buildErrorInfo(SF_ERR_DEPENDENCY_DOESNT_EXIST,
                               std::format(R"(Trying to add non-existing dependency "{}" to "{}")", dependent, id)));
        }
        _cellDependents[dependent].emplace_back(id);
    }
    setDirty(id);

    return {};
}

VoidResult Spreadsheet::removeCell(CellId const& id) {
    //_leaveCells.erase(id) //FIXME
    _cells.erase(id);
    //_dirtyLeaves.erase();
    //_cellDependencies.erase() + dependent from dependencies
    //_cellDependents.erase() + dependency from dependents

    return {};
}

VoidResult Spreadsheet::setCellValue(CellId const& id, CellValue value) {
    auto const& cell = _cells.find(id);
    if (cell == _cells.end()) [[unlikely]] {
        return std::unexpected(
            buildErrorInfo(SF_ERR_CELL_NOT_FOUND, std::format(R"(Trying to set value of non-existing cell "{}")", id)));
    }
    if (cell->second.value == value) [[unlikely]] {
        return std::unexpected(
            buildErrorInfo(SF_ERR_NOTHING_CHANGED, std::format(R"(No change to value of cell "{}")", id)));
    }

    cell->second.value = value;
    setDirty(id);
    return {};
}

CellValueResult Spreadsheet::getCellValue(CellId const& id) {
    auto const& cellEntry = _cells.find(id);
    if (cellEntry == _cells.end()) [[unlikely]] {
        return std::unexpected(buildErrorInfo(SF_ERR_CELL_NOT_FOUND,
                                              std::format(R"(Trying to read value of non-existing cell "{}")", id)));
    }

    const auto& cell = cellEntry->second;
    if (cell.dirty) {
        evaluate(id);
    }
    return cell.value;
}

void Spreadsheet::evaluate(CellId const& id) {
    (this->*evaluateImpl)(id);
}

VoidResult Spreadsheet::evaluate() {
    for (auto const& cell : _dirtyLeaves) {
        evaluate(cell);
    }
    _dirtyLeaves.clear();
    return {};
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
    if (!cell.dirty) {
        return;
    }

    for (auto const& dependency : _cellDependencies[id]) {
        evaluateRecursive(dependency);
    }

    cell.value = cell.formula();
    cell.dirty = false;
}

void Spreadsheet::evaluateIterative(CellId const& id) {
    if (!_cells[id].dirty) {
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

VoidResult Spreadsheet::wireDependencies(CellId const& id,
                                         std::unordered_set<std::string_view> const& depNames,
                                         bool skipCycleCheck) {
    std::vector<CellId> deps;
    deps.reserve(depNames.size());

    for (auto const& name : depNames) {
        deps.push_back(resolveCellId(name));
    }

    return setCellDependencies(id, deps, skipCycleCheck);
}

inline CellId Spreadsheet::resolveCellId(std::string_view const& id) {
    return std::string{id};
}

} // namespace statforge