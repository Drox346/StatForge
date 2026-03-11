#include "engine.hpp"
#include "error/error.h"

#include <cctype>
#include <vector>

namespace statforge::runtime {

namespace {

SF_ErrorCode extractErrorCode(VoidResult&& result) {
    if (result) {
        return SF_OK;
    }
    sf_set_error("%s", result.error().message.c_str());
    return result.error().errorCode;
}

std::vector<NodeId> parseDependencies(std::string_view dependencies) {
    std::vector<NodeId> parsed;
    std::string current;

    auto flushCurrent = [&parsed, &current]() {
        if (!current.empty()) {
            parsed.push_back(current);
            current.clear();
        }
    };

    for (unsigned char c : dependencies) {
        if (c == ',' || std::isspace(c)) {
            flushCurrent();
            continue;
        }
        current.push_back(static_cast<char>(c));
    }
    flushCurrent();

    return parsed;
}

} // namespace

EngineImpl::EngineImpl(statkernel::Executor::EvaluationType evaluationType) {
    ctx.kernel.setEvaluationType(evaluationType);
}

SF_ErrorCode EngineImpl::createAggregatorNode(NodeId const& name) {
    return extractErrorCode(ctx.kernel.createAggregatorNode(name, {}));
}

SF_ErrorCode EngineImpl::createFormulaNode(NodeId const& name, std::string_view formula) {
    return extractErrorCode(ctx.kernel.createFormulaNode(name, formula));
}

SF_ErrorCode EngineImpl::createValueNode(NodeId const& name, double value) {
    return extractErrorCode(ctx.kernel.createValueNode(name, value));
}

SF_ErrorCode EngineImpl::removeNode(NodeId const& name) {
    return extractErrorCode(ctx.kernel.removeNode(name));
}

SF_ErrorCode EngineImpl::setNodeValue(NodeId const& name, double value) {
    return extractErrorCode(ctx.kernel.setNodeValue(name, value));
}

SF_ErrorCode EngineImpl::setNodeFormula(NodeId const& name, std::string_view formula) {
    return extractErrorCode(ctx.kernel.setNodeFormula(name, formula));
}

SF_ErrorCode EngineImpl::setNodeDependency(NodeId const& name, std::string_view dependencies) {
    return extractErrorCode(ctx.kernel.setNodeDependencies(name, parseDependencies(dependencies)));
}

SF_ErrorCode EngineImpl::getNodeValue(NodeId const& name, double& value) {
    auto result = ctx.kernel.getNodeValue(name);
    if (!result) {
        sf_set_error("%s", result.error().message.c_str());
        return result.error().errorCode;
    }
    value = *result;
    return SF_OK;
}

std::string EngineImpl::getLastError() {
    return sf_last_error();
}

void EngineImpl::evaluate() {
    auto result = ctx.kernel.evaluate();
    if (!result) {
        sf_set_error("%s", result.error().message.c_str());
        return;
    }
}

void EngineImpl::reset() {
    ctx.reset();
}

} // namespace statforge::runtime
