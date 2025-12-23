#include "../test_util.hpp"
#include "error/error.h"
#include "stat_kernel/stat_kernel.hpp"

#include <doctest/doctest.h>
#include <string>

using namespace statforge;

TEST_CASE("create formula cell") {
    StatKernel kernel;
    const std::string cellNameVal1("value_cell1");
    const std::string cellNameVal2("value_cell2");
    const std::string cellNameFormula("formula_cell");
    CHECK(kernel.createValueCell(cellNameVal1, 1));
    CHECK(kernel.createValueCell(cellNameVal2, 2));
    CHECK(kernel.createFormulaCell(cellNameFormula, "<value_cell1> + <value_cell2>"));
    CHECK(kernel.evaluate());

    const auto cellValue = kernel.getCellValue(cellNameFormula);
    REQUIRE(cellValue);
    CHECK_EQ(*cellValue, 3.0);
}

TEST_CASE("basic spreadsheet setup") {
    StatKernel kernel;

    CHECK(kernel.createValueCell("FlatStrTree", 50));
    CHECK(kernel.createValueCell("FlatStrGear", 25));
    CHECK(kernel.createValueCell("IncStrTree", 30));
    CHECK(kernel.createValueCell("IncStrGear", 20));

    CHECK(kernel.createValueCell("BaseLife", 100));
    CHECK(kernel.createValueCell("FlatLifeLevel", 40));
    CHECK(kernel.createValueCell("FlatLifeGear", 20));
    CHECK(kernel.createValueCell("FlatLifeTree", 10));
    CHECK(kernel.createValueCell("LifePerStr", 1.5));
    CHECK(kernel.createValueCell("CharLevel", 40));

    CHECK(kernel.createValueCell("IncLifeGear", 10));
    CHECK(kernel.createValueCell("IncLifeTree", 15));
    CHECK(kernel.createValueCell("MoreLifeTree", 20));
    CHECK(kernel.createValueCell("MoreLifeGear", 10));

    CHECK(kernel.createAggregatorCell("Ag_FlatStr", {"FlatStrTree", "FlatStrGear"}));
    CHECK(kernel.createAggregatorCell("Ag_IncStr", {"IncStrTree", "IncStrGear"}));
    CHECK(
        kernel.createAggregatorCell("Ag_FlatLife",
                                    {"FlatLifeLevel", "FlatLifeGear", "FlatLifeTree", "BaseLife"}));
    CHECK(kernel.createAggregatorCell("Ag_IncLife", {"IncLifeGear", "IncLifeTree"}));
    CHECK(kernel.createAggregatorCell("Ag_MoreLife", {"MoreLifeGear", "MoreLifeTree"}));

    CHECK(kernel.createFormulaCell("FinalStr", "<Ag_FlatStr> * (1 + <Ag_IncStr> / 100)"));
    CHECK(kernel.createFormulaCell("FlatLifeStr", "<FinalStr> * <LifePerStr>"));
    CHECK(kernel.createFormulaCell(
        "FinalLife",
        "(<Ag_FlatLife> + <FlatLifeStr>) * (1 + <Ag_IncLife> / 100) * (1 + <Ag_MoreLife> / 100)"));

    CHECK(kernel.evaluate());

    const auto finalLife = kernel.getCellValue("FinalLife");
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
    StatKernel kernel;

    SUBCASE("duplicate cell") {
        CHECK(kernel.createValueCell("val", 2));
        CHECK(kernel.createFormulaCell("formula", "<val> + 2"));
        CHECK(kernel.createAggregatorCell("agg", {"formula"}));

        auto aggVal = kernel.getCellValue("agg");
        REQUIRE(aggVal);
        CHECK_EQ(*aggVal, 4);

        checkErrorCode(kernel.createValueCell("val", 5), SF_ERR_CELL_ALREADY_EXISTS);
        checkErrorCode(kernel.createFormulaCell("formula", "3 - <val>"),
                       SF_ERR_CELL_ALREADY_EXISTS);
        checkErrorCode(kernel.createAggregatorCell("agg", {"val"}), SF_ERR_CELL_ALREADY_EXISTS);

        aggVal = kernel.getCellValue("agg");
        REQUIRE(aggVal);
        CHECK_EQ(*aggVal, 4);
    }
    SUBCASE("dependency doesn't exit") {
        checkErrorCode(kernel.createFormulaCell("formula", "3 - <sdfdfgdfgdfg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
        checkErrorCode(kernel.createAggregatorCell("agg", {"fdshosadfojksdfoijksdrfff"}),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);

        checkErrorCode(kernel.getCellValue("formula"), SF_ERR_CELL_NOT_FOUND);
        checkErrorCode(kernel.getCellValue("agg"), SF_ERR_CELL_NOT_FOUND);
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(kernel.createFormulaCell("formula1", "3 - <<>"), SF_ERR_INVALID_DSL);
        checkErrorCode(kernel.getCellValue("formula"), SF_ERR_CELL_NOT_FOUND);
    }
}

TEST_CASE("cell manipulation errors") {
    StatKernel kernel;
    CHECK(kernel.createValueCell("value1", 50));
    CHECK(kernel.createValueCell("value2", 60));
    CHECK(kernel.createFormulaCell("formula1", "<value1>"));
    CHECK(kernel.createFormulaCell("formula2", "<value2>"));
    CHECK(kernel.createAggregatorCell("agg1", {"value1"}));
    CHECK(kernel.createAggregatorCell("agg2", {"value2"}));

    SUBCASE("cyclic dependencies") {
        constexpr size_t distance = 100;

        CHECK(kernel.createFormulaCell("f0", "<value1>"));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(kernel.createFormulaCell("f" + std::to_string(i),
                                           "<f" + std::to_string(i - 1) + ">"));
        }
        checkErrorCode(kernel.setCellFormula("f0", "<f" + std::to_string(distance - 1) + ">"),
                       SF_ERR_DEPENDENCY_LOOP);

        CHECK(kernel.createAggregatorCell("a0", {"value1"}));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(kernel.createAggregatorCell("a" + std::to_string(i),
                                              {"a" + std::to_string(i - 1)}));
        }
        checkErrorCode(kernel.setCellDependencies("a0", {"a" + std::to_string(distance - 1)}),
                       SF_ERR_DEPENDENCY_LOOP);
    }
    SUBCASE("self reference") {
        checkErrorCode(kernel.createFormulaCell("formula3", "<formula3>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(kernel.createAggregatorCell("agg3", {"agg3"}), SF_ERR_SELF_REFERENCE);

        checkErrorCode(kernel.setCellFormula("formula1", "<formula1>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(kernel.setCellDependencies("agg1", {"agg1"}), SF_ERR_SELF_REFERENCE);
    }
    SUBCASE("wrong cell types") {
        SUBCASE("setValue") {
            checkErrorCode(kernel.setCellValue("formula1", 10), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(kernel, "formula1", 50);

            checkErrorCode(kernel.setCellValue("agg1", 10), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(kernel, "agg1", 50);

            CHECK(kernel.setCellValue("value1", 10));
            checkValue(kernel, "value1", 10);
        }
        SUBCASE("setFormula") {
            checkErrorCode(kernel.setCellFormula("value1", "<value2>"), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(kernel, "value1", 50);

            checkErrorCode(kernel.setCellFormula("agg1", "<value2>"), SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(kernel, "agg1", 50);

            CHECK(kernel.setCellFormula("formula1", "<value2>"));
            checkValue(kernel, "formula1", 60);
        }
        SUBCASE("setDependencies") {
            checkErrorCode(kernel.setCellDependencies("value1", {"value2"}),
                           SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(kernel, "value1", 50);

            checkErrorCode(kernel.setCellDependencies("formula1", {"value2"}),
                           SF_ERR_CELL_TYPE_MISMATCH);
            checkValue(kernel, "formula1", 50);

            CHECK(kernel.setCellDependencies("agg1", {"value2"}));
            checkValue(kernel, "agg1", 60);
        }
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(kernel.setCellFormula("sdsflgjksdfjlksdfjlk", "1"), SF_ERR_CELL_NOT_FOUND);
        checkErrorCode(kernel.setCellFormula("formula1", "<>324werwe534dsf"), SF_ERR_INVALID_DSL);
        checkErrorCode(kernel.setCellFormula("formula1", "<dfdfsgggdfgdfsgdfsg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
    }
}