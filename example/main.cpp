#include "runtime/engine.hpp"

#include <iostream>

// [val] level: "47"
// [val] subclass: "1"
// [val] baselife: "150"
// [val] flatLifePerLevel: "48"

// [for] flatLifeSubclass: "<subclass> == 0 ? 0 :
//                         <subclass> == 1 ? 100 :
//                         <subclass> == 2 ? 300 : 0"
// [for] flatLifeLevel: "<level> * <flatLifePerLevel>"

// [agg] flatLifeAggr:
// dependency: baselife, flatLifeLevel, flatLifeSubclass

int main() {
    statforge::Engine engine;
    auto& spreadsheet = engine.kernel();

    spreadsheet.createValueCell("level", 47);
    spreadsheet.createValueCell("subclass", 47);
    spreadsheet.createValueCell("baselife", 47);
    spreadsheet.createValueCell("flatLifePerLevel", 47);
    spreadsheet.createFormulaCell(
        "flatLifeSubclass",
        "<subclass> == 0 ? 0 : <subclass> == 1 ? 100 : <subclass> == 2 ? 300 : 0");
    spreadsheet.createFormulaCell("flatLifeLevel", "<level> * <flatLifePerLevel>");
    spreadsheet.createAggregatorCell("flatLifeAggr",
                                     {"baselife", "flatLifeLevel", "flatLifeSubclass"});

    return 0;
}