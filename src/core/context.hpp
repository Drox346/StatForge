#pragma once

#include "spreadsheet/spreadsheet.hpp"
#include "state/rule_engine.hpp"

namespace statforge {

struct Context {
    Spreadsheet spreadsheet;
    RuleEngine ruleEngine;

    void reset() {
        spreadsheet.reset();
        ruleEngine.reset();
    }
};
    
} // namespace statforge