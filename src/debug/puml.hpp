#pragma once

#include "stat_kernel/cell.hpp"
#include "types/definitions.hpp"

namespace statforge::debug {

void logCells(std::string const& fileName,
              std::unordered_map<CellId, statkernel::Cell> const& cells,
              std::unordered_map<CellId, std::vector<CellId>> const& dependencyMap);

} // namespace statforge::debug
