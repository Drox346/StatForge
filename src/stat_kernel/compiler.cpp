#include "compiler.hpp"
#include "error/internal/error.hpp"

#include "dsl/parser.hpp"
#include "stat_kernel/cell.hpp"
#include "types/definitions.hpp"

#include <iostream>
#include <memory>
#include <string_view>

namespace {

constexpr bool skipCycleCheck = true;

}

namespace statforge::statkernel {

//addCell
//if(wireCell) -> ok
//else: removeCell()
VoidResult Compiler::addAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_graph.addCell(id, {}));

    // newly created cells cannot appear as dependencies of existing cells.
    // this guarantees skipCycleCheck is safe here.
    auto result = _graph.setCellDependencies(id, dependencies, skipCycleCheck);
    if (!result) {
        _graph.removeCell(id);
        return result;
    }

    auto aggregate = [this, id]() -> CellValue {
        CellValue value{0};

        const auto& dependencies = _graph.dependencies(id);
        for (auto const& dependency : dependencies) {
            value += _graph.cell(dependency).value;
        }

        return value;
    };

    _graph.cell(
        id) = {.formula = aggregate, .value = 0, .type = CellType::Aggregator, .dirty = true};

    return {};
}

VoidResult Compiler::addFormulaCell(CellId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_graph.addCell(id, {}));
    auto& cell = _graph.cell(id);
    cell = {.formula = {}, .value = 0.0, .type = CellType::Formula, .dirty = true};

    auto astResult = compileAst(id, formula);
    if (!astResult) {
        _graph.removeCell(id);
        return std::unexpected(std::move(astResult).error());
    }

    // newly created cells cannot appear as dependencies of existing cells.
    // this guarantees skipCycleCheck is safe here.
    auto dependencyResult =
        setCellDependencies(id, dsl::extractDependencies(astResult->expr), skipCycleCheck);
    if (!dependencyResult) {
        _graph.removeCell(id);
        return dependencyResult;
    }
    cell.formula = compileCellFormula(std::move(*astResult));

    return {};
}

VoidResult Compiler::addValueCell(CellId const& id, double value) {
    return _graph.addCell(
        id,
        {.formula = nullptr, .value = value, .type = CellType::Value, .dirty = false});
}

VoidResult Compiler::setCellFormula(CellId const& id, std::string_view formula) {
    SF_RETURN_UNEXPECTED_IF(!_graph.contains(id),
                            SF_ERR_CELL_NOT_FOUND,
                            std::format(R"(Trying to set formula of non-existing cell "{}")", id));
    SF_RETURN_UNEXPECTED_IF(
        _graph.contains(id) && (_graph.cell(id).type != CellType::Formula),
        SF_ERR_CELL_TYPE_MISMATCH,
        std::format(R"(Trying to manually change formula of non formula cell "{}")", id));

    auto astResult = compileAst(id, formula);
    SF_RETURN_ERROR_IF_UNEXPECTED(astResult);

    auto dependencyResult = setCellDependencies(id, dsl::extractDependencies(astResult->expr));
    SF_RETURN_ERROR_IF_UNEXPECTED(dependencyResult);

    _graph.cell(id).formula = compileCellFormula(std::move(*astResult));

    return {};
}

VoidResult Compiler::setAggCellDependencies(CellId const& id,
                                            std::vector<CellId> const& dependencies,
                                            bool skipCycleCheck) {
    SF_RETURN_UNEXPECTED_IF(
        _graph.contains(id) && (_graph.cell(id).type != CellType::Aggregator),
        SF_ERR_CELL_TYPE_MISMATCH,
        std::format(R"(Trying to manually change dependencies of non aggregator cell "{}")", id));

    return setCellDependencies(id, dependencies, skipCycleCheck);
}

VoidResult Compiler::setCellDependencies(CellId const& id,
                                         std::vector<CellId> const& dependencies,
                                         bool skipCycleCheck) {
    SF_RETURN_UNEXPECTED_IF(
        !_graph.contains(id),
        SF_ERR_CELL_NOT_FOUND,
        std::format(R"(Trying to set dependencies of non-existing cell "{}")", id));

    assert(_graph.cell(id).type != CellType::Value);

    return _graph.setCellDependencies(id, dependencies, skipCycleCheck);
}

Compiler::CompiledAstResult Compiler::compileAst(CellId const& id, std::string_view formula) {
    CompiledAst ast{};
    ast.source = std::make_unique<std::string>(formula);

    auto astResult = dsl::Tokenizer{*ast.source}
                         .tokenize()
                         .and_then([](auto const& tokens) { return dsl::Parser{tokens}.parse(); })
                         .transform_error([&id](auto&& error) {
                             error.message.insert(0, std::format(R"(Cell "{}": )", id));
                             return std::move(error);
                         });
    SF_RETURN_ERROR_IF_UNEXPECTED(astResult);

    ast.expr = std::move(*astResult.value());
    return ast;
}

CellFormula Compiler::compileCellFormula(CompiledAst compiledAst) {
    return [this, ast = std::make_shared<CompiledAst>(std::move(compiledAst))]() -> CellValue {
        dsl::Context ctx{.cellLookup = [this](std::string_view id) -> double {
            return _graph.cell(std::string(id)).value;
        }};
        return dsl::evaluate(ast->expr, ctx);
    };
}

} // namespace statforge::statkernel