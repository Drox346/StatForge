#pragma once

#include "stat_kernel/stat_kernel.hpp"
#include "state/rule_engine.hpp"

namespace statforge {

struct Context {
    StatKernel kernel;
    RuleEngine ruleEngine;

    void reset() {
        kernel.reset();
        ruleEngine.reset();
    }
};

} // namespace statforge