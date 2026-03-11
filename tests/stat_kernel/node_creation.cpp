#include "../test_util.hpp"
#include "error/error.h"
#include "stat_kernel/stat_kernel.hpp"

#include <doctest/doctest.h>
#include <string>

using namespace statforge;

TEST_CASE("create formula node") {
    StatKernel kernel;
    const std::string nodeNameVal1("value_node1");
    const std::string nodeNameVal2("value_node2");
    const std::string nodeNameFormula("formula_node");
    CHECK(kernel.createValueNode(nodeNameVal1, 1));
    CHECK(kernel.createValueNode(nodeNameVal2, 2));
    CHECK(kernel.createFormulaNode(nodeNameFormula, "<value_node1> + <value_node2>"));
    CHECK(kernel.evaluate());

    const auto nodeValue = kernel.getNodeValue(nodeNameFormula);
    REQUIRE(nodeValue);
    CHECK_EQ(*nodeValue, 3.0);
}

TEST_CASE("basic spreadsheet setup") {
    StatKernel kernel;

    CHECK(kernel.createValueNode("FlatStrTree", 50));
    CHECK(kernel.createValueNode("FlatStrGear", 25));
    CHECK(kernel.createValueNode("IncStrTree", 30));
    CHECK(kernel.createValueNode("IncStrGear", 20));

    CHECK(kernel.createValueNode("BaseLife", 100));
    CHECK(kernel.createValueNode("FlatLifeLevel", 40));
    CHECK(kernel.createValueNode("FlatLifeGear", 20));
    CHECK(kernel.createValueNode("FlatLifeTree", 10));
    CHECK(kernel.createValueNode("LifePerStr", 1.5));
    CHECK(kernel.createValueNode("CharLevel", 40));

    CHECK(kernel.createValueNode("IncLifeGear", 10));
    CHECK(kernel.createValueNode("IncLifeTree", 15));
    CHECK(kernel.createValueNode("MoreLifeTree", 20));
    CHECK(kernel.createValueNode("MoreLifeGear", 10));

    CHECK(kernel.createAggregatorNode("Ag_FlatStr", {"FlatStrTree", "FlatStrGear"}));
    CHECK(kernel.createAggregatorNode("Ag_IncStr", {"IncStrTree", "IncStrGear"}));
    CHECK(
        kernel.createAggregatorNode("Ag_FlatLife",
                                    {"FlatLifeLevel", "FlatLifeGear", "FlatLifeTree", "BaseLife"}));
    CHECK(kernel.createAggregatorNode("Ag_IncLife", {"IncLifeGear", "IncLifeTree"}));
    CHECK(kernel.createAggregatorNode("Ag_MoreLife", {"MoreLifeGear", "MoreLifeTree"}));

    CHECK(kernel.createFormulaNode("FinalStr", "<Ag_FlatStr> * (1 + <Ag_IncStr> / 100)"));
    CHECK(kernel.createFormulaNode("FlatLifeStr", "<FinalStr> * <LifePerStr>"));
    CHECK(kernel.createFormulaNode(
        "FinalLife",
        "(<Ag_FlatLife> + <FlatLifeStr>) * (1 + <Ag_IncLife> / 100) * (1 + <Ag_MoreLife> / 100)"));

    CHECK(kernel.evaluate());

    const auto finalLife = kernel.getNodeValue("FinalLife");
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

TEST_CASE("node creation errors") {
    StatKernel kernel;

    SUBCASE("duplicate node") {
        CHECK(kernel.createValueNode("val", 2));
        CHECK(kernel.createFormulaNode("formula", "<val> + 2"));
        CHECK(kernel.createAggregatorNode("agg", {"formula"}));

        auto aggVal = kernel.getNodeValue("agg");
        REQUIRE(aggVal);
        CHECK_EQ(*aggVal, 4);

        checkErrorCode(kernel.createValueNode("val", 5), SF_ERR_NODE_ALREADY_EXISTS);
        checkErrorCode(kernel.createFormulaNode("formula", "3 - <val>"),
                       SF_ERR_NODE_ALREADY_EXISTS);
        checkErrorCode(kernel.createAggregatorNode("agg", {"val"}), SF_ERR_NODE_ALREADY_EXISTS);

        aggVal = kernel.getNodeValue("agg");
        REQUIRE(aggVal);
        CHECK_EQ(*aggVal, 4);
    }
    SUBCASE("dependency doesn't exit") {
        checkErrorCode(kernel.createFormulaNode("formula", "3 - <sdfdfgdfgdfg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
        checkErrorCode(kernel.createAggregatorNode("agg", {"fdshosadfojksdfoijksdrfff"}),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);

        checkErrorCode(kernel.getNodeValue("formula"), SF_ERR_NODE_NOT_FOUND);
        checkErrorCode(kernel.getNodeValue("agg"), SF_ERR_NODE_NOT_FOUND);
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(kernel.createFormulaNode("formula1", "3 - <<>"), SF_ERR_INVALID_DSL);
        checkErrorCode(kernel.getNodeValue("formula"), SF_ERR_NODE_NOT_FOUND);
    }
}

TEST_CASE("node manipulation errors") {
    StatKernel kernel;
    CHECK(kernel.createValueNode("value1", 50));
    CHECK(kernel.createValueNode("value2", 60));
    CHECK(kernel.createFormulaNode("formula1", "<value1>"));
    CHECK(kernel.createFormulaNode("formula2", "<value2>"));
    CHECK(kernel.createAggregatorNode("agg1", {"value1"}));
    CHECK(kernel.createAggregatorNode("agg2", {"value2"}));

    SUBCASE("cyclic dependencies") {
        constexpr size_t distance = 100;

        CHECK(kernel.createFormulaNode("f0", "<value1>"));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(kernel.createFormulaNode("f" + std::to_string(i),
                                           "<f" + std::to_string(i - 1) + ">"));
        }
        checkErrorCode(kernel.setNodeFormula("f0", "<f" + std::to_string(distance - 1) + ">"),
                       SF_ERR_DEPENDENCY_LOOP);

        CHECK(kernel.createAggregatorNode("a0", {"value1"}));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(kernel.createAggregatorNode("a" + std::to_string(i),
                                              {"a" + std::to_string(i - 1)}));
        }
        checkErrorCode(kernel.setNodeDependencies("a0", {"a" + std::to_string(distance - 1)}),
                       SF_ERR_DEPENDENCY_LOOP);
    }
    SUBCASE("self reference") {
        checkErrorCode(kernel.createFormulaNode("formula3", "<formula3>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(kernel.createAggregatorNode("agg3", {"agg3"}), SF_ERR_SELF_REFERENCE);

        checkErrorCode(kernel.setNodeFormula("formula1", "<formula1>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(kernel.setNodeDependencies("agg1", {"agg1"}), SF_ERR_SELF_REFERENCE);
    }
    SUBCASE("wrong node types") {
        SUBCASE("setValue") {
            checkErrorCode(kernel.setNodeValue("formula1", 10), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "formula1", 50);

            checkErrorCode(kernel.setNodeValue("agg1", 10), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "agg1", 50);

            CHECK(kernel.setNodeValue("value1", 10));
            checkValue(kernel, "value1", 10);
        }
        SUBCASE("setFormula") {
            checkErrorCode(kernel.setNodeFormula("value1", "<value2>"), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "value1", 50);

            checkErrorCode(kernel.setNodeFormula("agg1", "<value2>"), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "agg1", 50);

            CHECK(kernel.setNodeFormula("formula1", "<value2>"));
            checkValue(kernel, "formula1", 60);
        }
        SUBCASE("setDependencies") {
            checkErrorCode(kernel.setNodeDependencies("value1", {"value2"}),
                           SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "value1", 50);

            checkErrorCode(kernel.setNodeDependencies("formula1", {"value2"}),
                           SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "formula1", 50);

            CHECK(kernel.setNodeDependencies("agg1", {"value2"}));
            checkValue(kernel, "agg1", 60);
        }
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(kernel.setNodeFormula("sdsflgjksdfjlksdfjlk", "1"), SF_ERR_NODE_NOT_FOUND);
        checkErrorCode(kernel.setNodeFormula("formula1", "<>324werwe534dsf"), SF_ERR_INVALID_DSL);
        checkErrorCode(kernel.setNodeFormula("formula1", "<dfdfsgggdfgdfsgdfsg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
    }
}