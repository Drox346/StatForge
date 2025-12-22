#pragma once

#include "types/definitions.hpp"

namespace statforge {

struct Rule {
    ActionType action;
    RuleValueType previousValue{};
};

} // namespace statforge