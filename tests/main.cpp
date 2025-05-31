#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "spreadsheet/spreadsheet.hpp"

using namespace doctest;
using namespace statforge;

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
    sheet.createAggregatorCell("Ag_FlatLife", {
        "FlatLifeLevel", "FlatLifeGear", "FlatLifeTree", "LifePerStr"
    });
    sheet.createAggregatorCell("Ag_IncLife", {"IncLifeGear", "IncLifeTree"});
    sheet.createAggregatorCell("Ag_MoreLife", {"MoreLifeGear", "MoreLifeTree"});

    sheet.createFormulaCell("FinalStr", "<Ag_FlatStr> * (1 + <Ag_IncStr> / 100)");
    sheet.createFormulaCell("FlatLifeStr", "<FinalStr> * <LifePerStr>");
    sheet.createFormulaCell("FinalLife", 
        "(<BaseLife> + <Ag_FlatLife> + <FlatLifeStr>) * (1 + <Ag_IncLife> / 100) * (1 + <Ag_MoreLife> / 100)");

    // Evaluate
    sheet.evaluate();

    REQUIRE(sheet.getCellValue("FinalStr") == Approx((50 + 25) * (1 + 50.0 / 100)).epsilon(0.001));
    REQUIRE(sheet.getCellValue("FinalLife") > 0);
}
