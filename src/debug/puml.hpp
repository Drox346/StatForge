#pragma once

#include "spreadsheet/cell.hpp"
#include "types/definitions.hpp"

namespace statforge::debug {

void logCells(std::string const& fileName,
              std::unordered_map<CellId, spreadsheet::Cell> const& cells,
              std::unordered_map<CellId, std::vector<CellId>> const& dependencyMap);

} // namespace statforge::debug
