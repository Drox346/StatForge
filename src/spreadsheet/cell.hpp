#pragma once

#include "common/internal/definitions.hpp"

#include <functional>

namespace statforge {

using CellFormula = std::function<CellValue()>;

enum class CellType : u_int8_t {
    Value,
    Formula,
    Aggregator,
};

struct Cell {
    CellFormula formula;
    bool dirty{false};
    CellValue value{};
    CellType type;
};

} // namespace statforge