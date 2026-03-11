#include "cpp.hpp"
#include "runtime/engine.hpp"

namespace statforge {

Engine::Engine() : _impl(std::make_unique<runtime::EngineImpl>()) {
}

Engine::~Engine() = default;

Engine::Engine(Engine&& other) noexcept = default;

Engine& Engine::operator=(Engine&& other) noexcept = default;

SF_ErrorCode Engine::createAggregatorNode(std::string const& name) {
    return _impl->createAggregatorNode(name);
}

SF_ErrorCode Engine::createFormulaNode(std::string const& name, std::string const& formula) {
    return _impl->createFormulaNode(name, formula);
}

SF_ErrorCode Engine::createValueNode(std::string const& name, double value) {
    return _impl->createValueNode(name, value);
}

SF_ErrorCode Engine::removeNode(std::string const& name) {
    return _impl->removeNode(name);
}

SF_ErrorCode Engine::setNodeValue(std::string const& name, double value) {
    return _impl->setNodeValue(name, value);
}

SF_ErrorCode Engine::setNodeFormula(std::string const& name, std::string const& formula) {
    return _impl->setNodeFormula(name, formula);
}

SF_ErrorCode Engine::setNodeDependency(std::string const& name, std::string const& dependencies) {
    return _impl->setNodeDependency(name, dependencies);
}

SF_ErrorCode Engine::getNodeValue(std::string const& name, double& value) const {
    return _impl->getNodeValue(name, value);
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
