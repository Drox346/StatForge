#pragma once

#include "common/internal/definitions.hpp"

namespace statforge {

struct Rule {
    ActionType action;
    RuleValueType previousValue{};
};
    
} // namespace statforge