#include "cpp.hpp"
#include "runtime/engine.hpp"

namespace statforge {

Engine::Engine() : _impl(std::make_unique<runtime::EngineImpl>()) {
}

Engine::~Engine() = default;

Engine::Engine(Engine&& other) noexcept = default;

Engine& Engine::operator=(Engine&& other) noexcept = default;

SF_ErrorCode Engine::createAggregatorCell(std::string const& name) {
    return _impl->createAggregatorCell(name);
}

SF_ErrorCode Engine::createFormulaCell(std::string const& name, std::string const& formula) {
    return _impl->createFormulaCell(name, formula);
}

SF_ErrorCode Engine::createValueCell(std::string const& name, double value) {
    return _impl->createValueCell(name, value);
}

SF_ErrorCode Engine::removeCell(std::string const& name) {
    return _impl->removeCell(name);
}

SF_ErrorCode Engine::setCellValue(std::string const& name, double value) {
    return _impl->setCellValue(name, value);
}

SF_ErrorCode Engine::setCellFormula(std::string const& name, std::string const& formula) {
    return _impl->setCellFormula(name, formula);
}

SF_ErrorCode Engine::setCellDependency(std::string const& name, std::string const& dependencies) {
    return _impl->setCellDependency(name, dependencies);
}

SF_ErrorCode Engine::getCellValue(std::string const& name, double& value) const {
    return _impl->getCellValue(name, value);
}

SF_ErrorCode Engine::createRule(std::string const& /*name*/,
                                std::string const& /*action*/,
                                double /*initValue*/) {
    // TODO: delegate to _impl
    return SF_ErrorCode{};
}

SF_ErrorCode Engine::editRule(std::string const& /*name*/,
                              std::string const& /*action*/,
                              double /*initValue*/) {
    // TODO: delegate to _impl
    return SF_ErrorCode{};
}

SF_ErrorCode Engine::deleteRule(std::string const& /*name*/) {
    // TODO: delegate to _impl
    return SF_ErrorCode{};
}

std::string Engine::getLastError() {
    return _impl->getLastError();
}

} // namespace statforge
