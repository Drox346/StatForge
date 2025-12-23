#include "engine.hpp"

namespace statforge::runtime {

Engine::Engine(statkernel::Executor::EvaluationType evaluationType) {
    ctx.kernel.setEvaluationType(evaluationType);
}

StatKernel& Engine::kernel() {
    return ctx.kernel;
}

RuleEngine& Engine::ruleEngine() {
    return ctx.ruleEngine;
}

void Engine::evaluate() {
}

void Engine::reset() {
    ctx.reset();
}

} // namespace statforge::runtime