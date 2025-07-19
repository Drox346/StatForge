#pragma once

#include <string>

namespace statforge {

class Engine;


/******* Engine ********/
Engine& createEngine();


/******* Cells ********/
void createAggregatorCell(Engine& engine, std::string const& name);
void createFormulaCell(Engine& engine, std::string const& name, std::string const& formula);
void createValueCell(Engine& engine, std::string const& name, double value);
void removeCell(Engine& engine, std::string& name);
void setCellValue(Engine& engine, double value);
void setCellFormula(Engine& engine, std::string const& name, std::string const& formula);
void setCellDependency(Engine& engine, std::string const& name, std::string const& dependencies);


/******* Rules ********/
void createRule(Engine& engine, std::string const& name, std::string const& action, double initValue);
void editRule(Engine& engine, std::string const& name, std::string const& action, double initValue);
void deleteRule(Engine& engine, std::string const& name);

} // namespace statforge