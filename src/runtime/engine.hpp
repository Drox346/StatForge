#pragma once

#include "runtime/context.hpp"

#include <string>

namespace statforge::runtime {

class EngineImpl {
public:
    EngineImpl(statkernel::Executor::EvaluationType evaluationType =
                   statkernel::Executor::EvaluationType::Iterative);

    SF_ErrorCode createAggregatorCell(CellId const& name);
    SF_ErrorCode createFormulaCell(CellId const& name, std::string_view formula);
    SF_ErrorCode createValueCell(CellId const& name, double value);
    SF_ErrorCode removeCell(CellId const& name);
    SF_ErrorCode setCellValue(CellId const& name, double value);
    SF_ErrorCode setCellFormula(CellId const& name, std::string_view formula);
    SF_ErrorCode setCellDependency(CellId const& name, std::string_view dependencies);
    SF_ErrorCode getCellValue(CellId const& name, double& value);
    std::string getLastError();

    void evaluate();
    void reset();

private:
    Context ctx;
};

} // namespace statforge::runtime
