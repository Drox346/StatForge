#include "stat_kernel.hpp"

#include "error/error.h"
#include "error/internal/error.hpp"
#include "stat_kernel/cell.hpp"
#include "types/definitions.hpp"

#include <cassert>
#include <format>
#include <string_view>

namespace statforge {

using namespace statkernel;

StatKernel::StatKernel() : _compiler(_graph), _executor(_graph) {
}

VoidResult StatKernel::createAggregatorCell(CellId const& id,
                                            std::vector<CellId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.addAggregatorCell(id, dependencies));
    _executor.markAsDirtyLeaf(id);

    return {};
}

VoidResult StatKernel::createFormulaCell(CellId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.addFormulaCell(id, formula));
    _executor.markAsDirtyLeaf(id);

    return {};
}

VoidResult StatKernel::createValueCell(CellId const& id, double value) {
    return _compiler.addValueCell(id, value);
}

VoidResult StatKernel::removeCell(CellId const& id) {
    if (auto result = _graph.removeCell(id); !result) [[unlikely]] {
        return result;
    }
    _executor.remove(id);

    return {};
}

VoidResult StatKernel::setCellValue(CellId const& id, CellValue value) {
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

VoidResult StatKernel::setCellFormula(CellId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.setCellFormula(id, formula));
    _executor.markDirty(id);

    return {};
}

VoidResult StatKernel::setCellDependencies(CellId const& id,
                                           std::vector<CellId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.setAggCellDependencies(id, dependencies));
    _executor.markDirty(id);

    return {};
}

CellValueResult StatKernel::getCellValue(CellId const& id) {
    return _executor.getCellValue(id);
}

VoidResult StatKernel::evaluate() {
    return _executor.evaluate();
}

void StatKernel::reset() {
    _graph.clear();
    _executor.reset();
}

void StatKernel::setEvaluationType(statkernel::Executor::EvaluationType evaluationType) {
    _executor.setEvaluationType(evaluationType);
}

} // namespace statforge