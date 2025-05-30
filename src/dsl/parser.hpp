#pragma once

#include "common/definitions.hpp"

namespace statforge {

class Parser {
public:
    static ActionType parseAction(const std::string& action);
    static FormulaType parseFormula(const std::string& formula);
    static std::vector<CellId> parseFormulaDependencies(const std::string& formula);
};
    
} // namespace statforge