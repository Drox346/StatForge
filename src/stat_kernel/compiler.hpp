#pragma once

#include "error/internal/error.hpp"
#include "types/definitions.hpp"
#include <dsl/evaluator.hpp>
#include <stat_kernel/cell.hpp>
#include <stat_kernel/graph.hpp>
#include <string_view>

namespace statforge::statkernel {

class Compiler {
public:
    Compiler() = delete;
    explicit Compiler(statkernel::Graph& graph) : _graph(graph) {
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
    static CompiledAstResult compileAst(CellId const& id, std::string_view formula);
    CellFormula compileCellFormula(CompiledAst ast);

    statkernel::Graph& _graph;
};

} // namespace statforge::statkernel