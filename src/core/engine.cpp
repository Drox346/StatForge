#include "engine.hpp"

namespace statforge::core {

Engine::Engine(Spreadsheet::EvaluationType evaluationType) {
    ctx.spreadsheet.setEvaluationType(evaluationType);
}

Spreadsheet& Engine::spreadsheet() {
    return ctx.spreadsheet;
}

RuleEngine& Engine::ruleEngine() {
    return ctx.ruleEngine;
}

void Engine::evaluate() {
}

void Engine::reset() {
    ctx.reset();
}

} // namespace statforge::core