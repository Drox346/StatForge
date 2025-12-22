#pragma once

#include "core/context.hpp"
#include "spreadsheet/spreadsheet.hpp"
#include "state/rule_engine.hpp"

namespace statforge::core {

class Engine {
public:
    Engine(spreadsheet::Executor::EvaluationType evaluationType =
               spreadsheet::Executor::EvaluationType::Iterative);

    Spreadsheet& spreadsheet();
    RuleEngine& ruleEngine();

    void evaluate();

    void reset();

private:
    Context ctx;
};

} // namespace statforge::core