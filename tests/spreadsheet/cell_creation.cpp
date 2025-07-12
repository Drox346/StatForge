#include "spreadsheet/spreadsheet.hpp"

#include <chrono>
#include <doctest/doctest.h>
#include <iostream>
#include <random>
#include <string>

using namespace statforge;

TEST_CASE("create formula cell") {
    Spreadsheet spreadsheet;
    const std::string cellNameVal1("value_cell1");
    const std::string cellNameVal2("value_cell2");
    const std::string cellNameFormula("formula_cell");
    spreadsheet.createValueCell(cellNameVal1, 1);
    spreadsheet.createValueCell(cellNameVal2, 2);
    spreadsheet.createFormulaCell(cellNameFormula, "<value_cell1> + <value_cell2>");
    spreadsheet.evaluate();

    CHECK_EQ(spreadsheet.getCellValue(cellNameFormula), 3.0);
}

TEST_CASE("basic spreadsheet setup") {
    Spreadsheet sheet;

    sheet.createValueCell("FlatStrTree", 50);
    sheet.createValueCell("FlatStrGear", 25);
    sheet.createValueCell("IncStrTree", 30);
    sheet.createValueCell("IncStrGear", 20);

    sheet.createValueCell("BaseLife", 100);
    sheet.createValueCell("FlatLifeLevel", 40);
    sheet.createValueCell("FlatLifeGear", 20);
    sheet.createValueCell("FlatLifeTree", 10);
    sheet.createValueCell("LifePerStr", 1.5);
    sheet.createValueCell("CharLevel", 40);

    sheet.createValueCell("IncLifeGear", 10);
    sheet.createValueCell("IncLifeTree", 15);
    sheet.createValueCell("MoreLifeTree", 20);
    sheet.createValueCell("MoreLifeGear", 10);

    sheet.createAggregatorCell("Ag_FlatStr", {"FlatStrTree", "FlatStrGear"});
    sheet.createAggregatorCell("Ag_IncStr", {"IncStrTree", "IncStrGear"});
    sheet.createAggregatorCell("Ag_FlatLife", {"FlatLifeLevel", "FlatLifeGear", "FlatLifeTree", "LifePerStr"});
    sheet.createAggregatorCell("Ag_IncLife", {"IncLifeGear", "IncLifeTree"});
    sheet.createAggregatorCell("Ag_MoreLife", {"MoreLifeGear", "MoreLifeTree"});

    sheet.createFormulaCell("FinalStr", "<Ag_FlatStr> * (1 + <Ag_IncStr> / 100)");
    sheet.createFormulaCell("FlatLifeStr", "<FinalStr> * <LifePerStr>");
    sheet.createFormulaCell(
        "FinalLife",
        "(<BaseLife> + <Ag_FlatLife> + <FlatLifeStr>) * (1 + <Ag_IncLife> / 100) * (1 + <Ag_MoreLife> / 100)");

    sheet.evaluate();

    CHECK_EQ(sheet.getCellValue("FinalStr"), doctest::Approx((50 + 25) * (1 + 50.0 / 100)).epsilon(0.001));
    CHECK(sheet.getCellValue("FinalLife") > 0);
}