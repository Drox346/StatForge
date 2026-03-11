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

    /******* Nodes ********/
    SF_ErrorCode createCollectionNode(std::string const& name);
    SF_ErrorCode createFormulaNode(std::string const& name, std::string const& formula);
    SF_ErrorCode createValueNode(std::string const& name, double value);
    SF_ErrorCode removeNode(std::string const& name);
    SF_ErrorCode setNodeValue(std::string const& name, double value);
    SF_ErrorCode setNodeFormula(std::string const& name, std::string const& formula);
    SF_ErrorCode setNodeDependency(std::string const& name, std::string const& dependencies);
    SF_ErrorCode getNodeValue(std::string const& name, double& value) const;

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
