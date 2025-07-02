#pragma once

#include "common/definitions.hpp"

#include <functional>

namespace statforge {

using CellFormula = std::function<CellValue()>;

struct Cell {
    CellFormula formula;
    bool dirty{false};
    CellValue value{};
};

} // namespace statforge