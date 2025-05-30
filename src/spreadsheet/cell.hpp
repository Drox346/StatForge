#pragma once

#include "common/definitions.hpp"

#include <functional>

namespace statforge {

struct Cell {
    std::function<CellValue()> formula;
    bool dirty{false};
    CellValue value{};
};
    
} // namespace statforge