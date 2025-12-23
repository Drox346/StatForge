#include "../test_util.hpp"
#include "error/error.h"
#include "spreadsheet/spreadsheet.hpp"

#include <doctest/doctest.h>
#include <string>

using namespace statforge;

TEST_CASE("create formula cell") {
    Spreadsheet spreadsheet;
    const std::string cellNameVal1("value_cell1");
    const std::string cellNameVal2("value_cell2");
    const std::string cellNameFormula("formula_cell");
    CHECK(spreadsheet.createValueCell(cellNameVal1, 1));
    CHECK(spreadsheet.createValueCell(cellNameVal2, 2));
    CHECK(spreadsheet.createFormulaCell(cellNameFormula, "<value_cell1> + <value_cell2>"));
    CHECK(spreadsheet.evaluate());

    const auto cellValue = spreadsheet.getCellValue(cellNameFormula);
    REQUIRE(cellValue);
    CHECK_EQ(*cellValue, 3.0);
}

TEST_CASE("basic spreadsheet setup") {
    Spreadsheet sheet;

    CHECK(sheet.createValueCell("FlatStrTree", 50));
    CHECK(sheet.createValueCell("FlatStrGear", 25));
    CHECK(sheet.createValueCell("IncStrTree", 30));
    CHECK(sheet.createValueCell("IncStrGear", 20));

    CHECK(sheet.createValueCell("BaseLife", 100));
    CHECK(sheet.createValueCell("FlatLifeLevel", 40));
    CHECK(sheet.createValueCell("FlatLifeGear", 20));
    CHECK(sheet.createValueCell("FlatLifeTree", 10));
    CHECK(sheet.createValueCell("LifePerStr", 1.5));
    CHECK(sheet.createValueCell("CharLevel", 40));

    CHECK(sheet.createValueCell("IncLifeGear", 10));
    CHECK(sheet.createValueCell("IncLifeTree", 15));
    CHECK(sheet.createValueCell("MoreLifeTree", 20));
    CHECK(sheet.createValueCell("MoreLifeGear", 10));

    CHECK(sheet.createAggregatorCell("Ag_FlatStr", {"FlatStrTree", "FlatStrGear"}));
    CHECK(sheet.createAggregatorCell("Ag_IncStr", {"IncStrTree", "IncStrGear"}));
    CHECK(
        sheet.createAggregatorCell("Ag_FlatLife",
                                   {"FlatLifeLevel", "FlatLifeGear", "FlatLifeTree", "BaseLife"}));
    CHECK(sheet.createAggregatorCell("Ag_IncLife", {"IncLifeGear", "IncLifeTree"}));
    CHECK(sheet.createAggregatorCell("Ag_MoreLife", {"MoreLifeGear", "MoreLifeTree"}));

    CHECK(sheet.createFormulaCell("FinalStr", "<Ag_FlatStr> * (1 + <Ag_IncStr> / 100)"));
    CHECK(sheet.createFormulaCell("FlatLifeStr", "<FinalStr> * <LifePerStr>"));
    CHECK(sheet.createFormulaCell(
        "FinalLife",
        "(<Ag_FlatLife> + <FlatLifeStr>) * (1 + <Ag_IncLife> / 100) * (1 + <Ag_MoreLife> / 100)"));

    CHECK(sheet.evaluate());

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
        checkErrorCode(sheet.createFormulaCell("formula", "3 - <sdfdfgdfgdfg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
        checkErrorCode(sheet.createAggregatorCell("agg", {"fdshosadfojksdfoijksdrfff"}),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);

        checkErrorCode(sheet.getCellValue("formula"), SF_ERR_CELL_NOT_FOUND);
        checkErrorCode(sheet.getCellValue("agg"), SF_ERR_CELL_NOT_FOUND);
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(sheet.createFormulaCell("formula1", "3 - <<>"), SF_ERR_INVALID_DSL);
        checkErrorCode(sheet.getCellValue("formula"), SF_ERR_CELL_NOT_FOUND);
    }
}

TEST_CASE("cell manipulation errors") {
    Spreadsheet sheet;
    CHECK(sheet.createValueCell("value1", 50));
    CHECK(sheet.createValueCell("value2", 60));
    CHECK(sheet.createFormulaCell("formula1", "<value1>"));
    CHECK(sheet.createFormulaCell("formula2", "<value2>"));
    CHECK(sheet.createAggregatorCell("agg1", {"value1"}));
    CHECK(sheet.createAggregatorCell("agg2", {"value2"}));

    SUBCASE("cyclic dependencies") {
        constexpr size_t distance = 100;

        CHECK(sheet.createFormulaCell("f0", "<value1>"));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(sheet.createFormulaCell("f" + std::to_string(i),
                                          "<f" + std::to_string(i - 1) + ">"));
        }
        checkErrorCode(sheet.setCellFormula("f0", "<f" + std::to_string(distance - 1) + ">"),
                       SF_ERR_DEPENDENCY_LOOP);

        CHECK(sheet.createAggregatorCell("a0", {"value1"}));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(
                sheet.createAggregatorCell("a" + std::to_string(i), {"a" + std::to_string(i - 1)}));
        }
        checkErrorCode(sheet.setCellDependencies("a0", {"a" + std::to_string(distance - 1)}),
                       SF_ERR_DEPENDENCY_LOOP);
    }
    SUBCASE("self reference") {
        checkErrorCode(sheet.createFormulaCell("formula3", "<formula3>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(sheet.createAggregatorCell("agg3", {"agg3"}), SF_ERR_SELF_REFERENCE);

        checkErrorCode(sheet.setCellFormula("formula1", "<formula1>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(sheet.setCellDependencies("agg1", {"agg1"}), SF_ERR_SELF_REFERENCE);
    }
    SUBCASE("wrong cell types") {
        SUBCASE("setValue") {
            checkErrorCode(sheet.setCellValue("formula1", 10), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(sheet, "formula1", 50);

            checkErrorCode(sheet.setCellValue("agg1", 10), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(sheet, "agg1", 50);

            CHECK(sheet.setCellValue("value1", 10));
            checkValue(sheet, "value1", 10);
        }
        SUBCASE("setFormula") {
            checkErrorCode(sheet.setCellFormula("value1", "<value2>"), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(sheet, "value1", 50);

            checkErrorCode(sheet.setCellFormula("agg1", "<value2>"), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(sheet, "agg1", 50);

            CHECK(sheet.setCellFormula("formula1", "<value2>"));
            checkValue(sheet, "formula1", 60);
        }
        SUBCASE("setDependencies") {
            checkErrorCode(sheet.setCellDependencies("value1", {"value2"}),
                           SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(sheet, "value1", 50);

            checkErrorCode(sheet.setCellDependencies("formula1", {"value2"}),
                           SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(sheet, "formula1", 50);

            CHECK(sheet.setCellDependencies("agg1", {"value2"}));
            checkValue(sheet, "agg1", 60);
        }
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(sheet.setCellFormula("sdsflgjksdfjlksdfjlk", "1"), SF_ERR_CELL_NOT_FOUND);
        checkErrorCode(sheet.setCellFormula("formula1", "<>324werwe534dsf"), SF_ERR_INVALID_DSL);
        checkErrorCode(sheet.setCellFormula("formula1", "<dfdfsgggdfgdfsgdfsg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
    }
}