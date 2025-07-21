#pragma once

#include <cstddef>
#include <functional>
#include <string>

namespace statforge {

using CellId = std::string;
using CellValue = double;
using FormulaType = std::function<CellValue()>;

using RuleId = std::string;
using RuleValueType = double;
using ActionType = std::function<void(RuleValueType)>;


constexpr std::size_t maxCellCount{50000};

} // namespace statforge