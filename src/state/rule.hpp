#pragma once

#include "common/definitions.hpp"

namespace statforge {

struct Rule {
    ActionType action;
    RuleValueType previousValue{};
};
    
} // namespace statforge