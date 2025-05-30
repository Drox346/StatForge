#pragma once

#include "core/context.hpp"
#include "spreadsheet/spreadsheet.hpp"
#include "state/rule_engine.hpp"

namespace statforge {

class Engine {
public:
    Engine(Spreadsheet::EvaluationType evaluationType = Spreadsheet::EvaluationType::Iterative);

    Spreadsheet& spreadsheet();
    RuleEngine& ruleEngine();

    void evaluate();
    
    void reset();

private:
    Context ctx;
};
    
} // namespace statforge