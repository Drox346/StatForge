#pragma once

#include "error/internal/error.hpp"
#include "spreadsheet/cell.hpp"
#include "types/definitions.hpp"

namespace statforge::spreadsheet {

class Graph {
public:
    [[nodiscard]] bool contains(CellId const& id) const;

    // IMPPORTANT: For performance reasons calling "cell()" on a non-existing cell is
    // invalid and will terminate the program. Call contains() first if necessary.
    [[nodiscard]] Cell& cell(CellId const& id);
    [[nodiscard]] Cell const& cell(CellId const& id) const;

    [[nodiscard]] std::vector<CellId> const& dependencies(CellId const& id) const;
    [[nodiscard]] std::vector<CellId> const& dependents(CellId const& id) const;

    VoidResult addCell(CellId id, Cell cell);
    VoidResult setCellDependencies(CellId id,
                                   std::vector<CellId> deps,
                                   bool skipCycleCheck = false);
    VoidResult removeCell(CellId const& id);

    void clear();

private:
    [[nodiscard]] std::vector<CellId>& dependents(CellId const& id);

    std::unordered_map<CellId, Cell> _cells;
    std::unordered_map<CellId, std::vector<CellId>> _dependenciesMap;
    std::unordered_map<CellId, std::vector<CellId>> _dependentsMap;
};

} // namespace statforge::spreadsheet