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

/**
 * @brief Set the cell value. Only use it on value cells.
 *        Non value cells are converted to value cells.
 * 
 * @param engine 
 * @param value 
 */
void setCellValue(Engine& engine, double value);

/**
 * @brief Set the cell formula. Only use it on formula cell.
 *        Non formula cells are converted to formula cells.
 * 
 * @param engine 
 * @param name
 * @param formula 
 */
void setCellFormula(Engine& engine, std::string const& name, std::string const& formula);


/******* Rules ********/
void createRule(Engine& engine, std::string const& name, std::string const& action, double initValue);
void editRule(Engine& engine, std::string const& name, std::string const& action, double initValue);
void deleteRule(Engine& engine, std::string const& name);
} // namespace statforge