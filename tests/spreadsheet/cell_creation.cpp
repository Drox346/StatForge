#include "../test_util.hpp"
#include "spreadsheet/spreadsheet.hpp"

#include <doctest/doctest.h>
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

    const auto cellValue = spreadsheet.getCellValue(cellNameFormula);
    REQUIRE(cellValue);
    CHECK_EQ(*cellValue, 3.0);
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
    sheet.createAggregatorCell("Ag_FlatLife", {"FlatLifeLevel", "FlatLifeGear", "FlatLifeTree", "BaseLife"});
    sheet.createAggregatorCell("Ag_IncLife", {"IncLifeGear", "IncLifeTree"});
    sheet.createAggregatorCell("Ag_MoreLife", {"MoreLifeGear", "MoreLifeTree"});

    sheet.createFormulaCell("FinalStr", "<Ag_FlatStr> * (1 + <Ag_IncStr> / 100)");
    sheet.createFormulaCell("FlatLifeStr", "<FinalStr> * <LifePerStr>");
    sheet.createFormulaCell("FinalLife",
                            "(<Ag_FlatLife> + <FlatLifeStr>) * (1 + <Ag_IncLife> / 100) * (1 + <Ag_MoreLife> / 100)");

    sheet.evaluate();

    const auto finalLife = sheet.getCellValue("FinalLife");
    REQUIRE(finalLife);

    constexpr double Ag_FlatStr = 50.0 + 25.0;
    constexpr double Ag_IncStr = 30.0 + 20.0;
    constexpr double Ag_FlatLife = 100.0 + 40.0 + 20.0 + 10.0;
    constexpr double Ag_IncLife = 10.0 + 15.0;
    constexpr double Ag_MoreLife = 20.0 + 10.0;
    constexpr double FinalStr = Ag_FlatStr * (1.0 + Ag_IncStr / 100.0);
    constexpr double FlatLifeStr = FinalStr * 1.5;
    constexpr double expectedFinalLife =
        (Ag_FlatLife + FlatLifeStr) * (1.0 + Ag_IncLife / 100.0) * (1.0 + Ag_MoreLife / 100.0);

    CHECK_EQ(*finalLife, doctest::Approx(expectedFinalLife).epsilon(0.001));
}

TEST_CASE("cell creation errors") {
    Spreadsheet sheet;

    SUBCASE("duplicate cell") {
        CHECK(sheet.createValueCell("val", 2));
        CHECK(sheet.createFormulaCell("formula", "<val> + 2"));
        CHECK(sheet.createAggregatorCell("agg", {"formula"}));

        auto aggVal = sheet.getCellValue("agg");
        REQUIRE(aggVal);
        CHECK_EQ(*aggVal, 4);

        checkErrorCode(sheet.createValueCell("val", 5), SF_ERR_CELL_ALREADY_EXISTS);
        checkErrorCode(sheet.createFormulaCell("formula", "3 - <val>"), SF_ERR_CELL_ALREADY_EXISTS);
        checkErrorCode(sheet.createAggregatorCell("agg", {"val"}), SF_ERR_CELL_ALREADY_EXISTS);

        aggVal = sheet.getCellValue("agg");
        REQUIRE(aggVal);
        CHECK_EQ(*aggVal, 4);
    }
    SUBCASE("dependency doesn't exit") {
        checkErrorCode(sheet.createFormulaCell("formula", "3 - <sdfdfgdfgdfg>"), SF_ERR_DEPENDENCY_DOESNT_EXIST);
        checkErrorCode(sheet.createAggregatorCell("agg", {"fdshosadfojksdfoijksdrfff"}),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);

        checkErrorCode(sheet.getCellValue("formula"), SF_ERR_CELL_NOT_FOUND);
        checkErrorCode(sheet.getCellValue("agg"), SF_ERR_CELL_NOT_FOUND);

        auto asdsad = sheet.getCellValue("agg");
        int sadasd{};
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(sheet.createFormulaCell("formula1", "3 - <<>"), SF_ERR_INVALID_DSL);
        checkErrorCode(sheet.getCellValue("formula"), SF_ERR_CELL_NOT_FOUND);
    }
}

TEST_CASE("cell manipulation errors") {
    Spreadsheet sheet;

    SUBCASE("cyclic dependencies") {
        // TODO
        // - changeCellFormula (formula)
        // - setDependencies (agg)
    }
    SUBCASE("self reference") {
        // TODO
        // - changeCellFormula (formula)
        // - setDependencies (agg)
    }
    SUBCASE("value of cell hasnt changed") {
        CHECK(sheet.createValueCell("val", 3));

        const auto val = sheet.getCellValue("val");
        REQUIRE(val);
        CHECK_EQ(*val, 3);

        checkErrorCode(sheet.setCellValue("val", 3), SF_ERR_NOTHING_CHANGED);

        const auto aggVal = sheet.getCellValue("val");
        REQUIRE(val);
        CHECK_EQ(*val, 3);
    }
    SUBCASE("ill-formed dsl") {
        // TODO changeCellFormula
    }
}

#include <chrono>
#include <iostream>
#include <random>
TEST_CASE("benchmark cell dependency evaluation") {
    constexpr size_t cellTarget = 100'000 / 2;
    size_t cells = 0;
    Spreadsheet sheet;

    sheet.createValueCell("root", 1);
    sheet.createFormulaCell("a0", "<root>");
    size_t lastRowCount = 1;

    auto t6 = std::chrono::steady_clock::now();
    for (size_t i = 0; i < cellTarget;) {

        for (size_t j = i + 1; j - (i + 1) != lastRowCount;) {
            sheet.createFormulaCell("a" + std::to_string(j), "root(2, <a" + std::to_string(i) + ">) + 1");
            j++;
            sheet.createFormulaCell("a" + std::to_string(j), "root(2, <a" + std::to_string(i) + ">) + 1");
            j++;
            i++;

            cells = j;
        }
        lastRowCount *= 2;
    }
    auto t7 = std::chrono::steady_clock::now();

    std::cout << "cells: " << cells << "\n";

    // initial clear dirty
    auto t0 = std::chrono::steady_clock::now();
    sheet.evaluate();
    auto t1 = std::chrono::steady_clock::now();

    // dirty propagation
    auto t2 = std::chrono::steady_clock::now();
    sheet.setCellValue("root", 2);
    auto t3 = std::chrono::steady_clock::now();

    // update all cells
    auto t4 = std::chrono::steady_clock::now();
    sheet.evaluate();
    auto t5 = std::chrono::steady_clock::now();
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<std::mt19937::result_type> dist(1, 200);
    auto t8 = std::chrono::steady_clock::now();
    // for (size_t i = 0; i < 2000; ++i) {
    //     sheet.setCellValue("root", dist(rng));
    //     sheet.evaluate();
    // }
    auto t9 = std::chrono::steady_clock::now();


    auto ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    auto ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count();
    auto ms4 = std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6).count();
    auto ms5 = std::chrono::duration_cast<std::chrono::milliseconds>(t9 - t8).count();
    std::cout << "Creation of " << cells << " formula cells: " << ms4 << "ms\n"
              << "Initial evaluate after creation: " << ms1 << "ms\n"
              << "Setting " << cells << " cells to dirty: " << ms2 << "ms\n"
              << "Evaluate " << cells << " dirty cells: " << ms3 << "ms\n"
        //<< "2000x random values + recalcs: " << ms5 << "ms\n"
        ;
}