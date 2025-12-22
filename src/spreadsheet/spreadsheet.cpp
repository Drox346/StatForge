#include "spreadsheet.hpp"

#include "error/error.h"
#include "error/internal/error.hpp"
#include "spreadsheet/cell.hpp"
#include "types/definitions.hpp"

#include <cassert>
#include <cstdint>
#include <format>
#include <stack>
#include <string_view>

namespace statforge {

using namespace spreadsheet;

Spreadsheet::Spreadsheet() : _compiler(_graph), _executor(_graph) {
}

VoidResult Spreadsheet::createAggregatorCell(CellId const& id,
                                             std::vector<CellId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.addAggregatorCell(id, dependencies));
    _executor.markAsDirtyLeaf(id);

    return {};
}

VoidResult Spreadsheet::createFormulaCell(CellId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.addFormulaCell(id, formula));
    _executor.markAsDirtyLeaf(id);

    return {};
}

VoidResult Spreadsheet::createValueCell(CellId const& id, double value) {
    return _compiler.addValueCell(id, value);
}

VoidResult Spreadsheet::removeCell(CellId const& id) {
    if (auto result = _graph.removeCell(id); !result) [[unlikely]] {
        return result;
    }
    _executor.remove(id);

    return {};
}

VoidResult Spreadsheet::setCellValue(CellId const& id, CellValue value) {
    SF_RETURN_UNEXPECTED_IF(!_graph.contains(id),
                            SF_ERR_CELL_NOT_FOUND,
                            std::format(R"(Trying to set value of non-existing cell "{}")", id));

    auto& cell = _graph.cell(id);
    SF_RETURN_UNEXPECTED_IF(cell.type != CellType::Value,
                            SF_ERR_CELL_TYPE_MISMATCH,
                            std::format(R"(Trying to change value of non value cell "{}")", id));

    if (cell.value == value) {
        return {};
    }

    cell.value = value;
    _executor.markDirty(id);
    return {};
}

VoidResult Spreadsheet::setCellFormula(CellId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.setCellFormula(id, formula));
    _executor.markDirty(id);

    return {};
}

VoidResult Spreadsheet::setCellDependencies(CellId const& id,
                                            std::vector<CellId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.setAggCellDependencies(id, dependencies));
    _executor.markDirty(id);

    return {};
}

CellValueResult Spreadsheet::getCellValue(CellId const& id) {
    return _executor.getCellValue(id);
}

VoidResult Spreadsheet::evaluate() {
    return _executor.evaluate();
}

void Spreadsheet::reset() {
    _graph.clear();
    _executor.reset();
}

void Spreadsheet::setEvaluationType(spreadsheet::Executor::EvaluationType evaluationType) {
    _executor.setEvaluationType(evaluationType);
}

} // namespace statforge