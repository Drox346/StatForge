#pragma once

#include "runtime/context.hpp"
#include "stat_kernel/stat_kernel.hpp"
#include "state/rule_engine.hpp"

namespace statforge::runtime {

class Engine {
public:
    Engine(statkernel::Executor::EvaluationType evaluationType =
               statkernel::Executor::EvaluationType::Iterative);

    StatKernel& kernel();
    RuleEngine& ruleEngine();

    void evaluate();

    void reset();

private:
    Context ctx;
};

} // namespace statforge::runtime