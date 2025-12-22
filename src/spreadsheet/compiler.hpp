#pragma once

#include "dsl/parser.hpp"
#include "error/internal/error.hpp"
#include "types/definitions.hpp"
#include <dsl/evaluator.hpp>
#include <spreadsheet/cell.hpp>
#include <spreadsheet/graph.hpp>
#include <string_view>

namespace statforge::spreadsheet {

class Compiler {
public:
    Compiler() = delete;
    explicit Compiler(spreadsheet::Graph& graph) : _graph(graph) {
    }

    VoidResult addAggregatorCell(CellId const& id, std::vector<CellId> const& dependencies);
    VoidResult addFormulaCell(CellId const& id, std::string_view formula);
    VoidResult addValueCell(CellId const& id, double value);

    VoidResult setCellFormula(CellId const& id, std::string_view formula);
    VoidResult setAggCellDependencies(CellId const& id,
                                      std::vector<CellId> const& dependencies,
                                      bool skipCycleCheck = false);

private:
    VoidResult setCellDependencies(CellId const& id,
                                   std::vector<CellId> const& dependencies,
                                   bool skipCycleCheck = false);

    struct CompiledAst {
        // store "source" behind pointer to guarantee that copying or
        // moving this struct wont break string views referencing it
        std::unique_ptr<std::string> source;
        dsl::ExpressionTree expr;
    };
    using CompiledAstResult = Result<CompiledAst>;
    CompiledAstResult compileAst(CellId const& id, std::string_view formula);
    CellFormula compileCellFormula(CompiledAst ast);

    [[maybe_unused]] spreadsheet::Graph& _graph;
};

} // namespace statforge::spreadsheet