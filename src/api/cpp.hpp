#pragma once

#include "error/error.h"

#include <memory>
#include <string>

namespace statforge {
namespace runtime {
class EngineImpl;
}

class Engine {
public:
    Engine();
    ~Engine();
    Engine(Engine&&) noexcept;
    Engine& operator=(Engine&&) noexcept;
    Engine(Engine const&) = delete;
    Engine& operator=(Engine const&) = delete;

    /******* Cells ********/
    SF_ErrorCode createAggregatorCell(std::string const& name);
    SF_ErrorCode createFormulaCell(std::string const& name, std::string const& formula);
    SF_ErrorCode createValueCell(std::string const& name, double value);
    SF_ErrorCode removeCell(std::string const& name);
    SF_ErrorCode setCellValue(std::string const& name, double value);
    SF_ErrorCode setCellFormula(std::string const& name, std::string const& formula);
    SF_ErrorCode setCellDependency(std::string const& name, std::string const& dependencies);
    SF_ErrorCode getCellValue(std::string const& name, double& value) const;

    /******* Rules ********/
    SF_ErrorCode createRule(std::string const& name, std::string const& action, double initValue);
    SF_ErrorCode editRule(std::string const& name, std::string const& action, double initValue);
    SF_ErrorCode deleteRule(std::string const& name);

    /******* Error ********/
    std::string getLastError();

private:
    std::unique_ptr<runtime::EngineImpl> _impl;
};

} // namespace statforge
