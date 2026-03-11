#pragma once

#include <functional>
#include <string>

namespace statforge {

using NodeId = std::string;
using NodeValue = double;
using FormulaType = std::function<NodeValue()>;

using RuleId = std::string;
using RuleValueType = double;
using ActionType = std::function<void(RuleValueType)>;

} // namespace statforge