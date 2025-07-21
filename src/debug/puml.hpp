#pragma once

#include "common/internal/definitions.hpp"
#include "spreadsheet/cell.hpp"

namespace statforge::debug {

void logCells(std::string const& fileName,
              std::unordered_map<CellId, Cell> const& cells,
              std::unordered_map<CellId, std::vector<CellId>> const& dependencyMap);

} // namespace statforge::debug
