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

std::vector<CellId> parseDependencies(std::string_view dependencies) {
    std::vector<CellId> parsed;
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

SF_ErrorCode EngineImpl::createAggregatorCell(CellId const& name) {
    return extractErrorCode(ctx.kernel.createAggregatorCell(name, {}));
}

SF_ErrorCode EngineImpl::createFormulaCell(CellId const& name, std::string_view formula) {
    return extractErrorCode(ctx.kernel.createFormulaCell(name, formula));
}

SF_ErrorCode EngineImpl::createValueCell(CellId const& name, double value) {
    return extractErrorCode(ctx.kernel.createValueCell(name, value));
}

SF_ErrorCode EngineImpl::removeCell(CellId const& name) {
    return extractErrorCode(ctx.kernel.removeCell(name));
}

SF_ErrorCode EngineImpl::setCellValue(CellId const& name, double value) {
    return extractErrorCode(ctx.kernel.setCellValue(name, value));
}

SF_ErrorCode EngineImpl::setCellFormula(CellId const& name, std::string_view formula) {
    return extractErrorCode(ctx.kernel.setCellFormula(name, formula));
}

SF_ErrorCode EngineImpl::setCellDependency(CellId const& name, std::string_view dependencies) {
    return extractErrorCode(ctx.kernel.setCellDependencies(name, parseDependencies(dependencies)));
}

SF_ErrorCode EngineImpl::getCellValue(CellId const& name, double& value) {
    auto result = ctx.kernel.getCellValue(name);
    if (!result) {
        sf_set_error("%s", result.error().message.c_str());
        return result.error().errorCode;
    }
    sf_set_error("");
    value = *result;
    return SF_OK;
}

std::string EngineImpl::getLastError() {
    return sf_last_error();
}

void EngineImpl::evaluate() {
}

void EngineImpl::reset() {
    ctx.reset();
}

} // namespace statforge::runtime
