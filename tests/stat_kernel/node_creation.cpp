#include "../test_util.hpp"
#include "api/cpp.hpp"
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

    CHECK(kernel.createCollectionNode("Col_FlatStr", {"FlatStrTree", "FlatStrGear"}));
    CHECK(kernel.createCollectionNode("Col_IncStr", {"IncStrTree", "IncStrGear"}));
    CHECK(
        kernel.createCollectionNode("Col_FlatLife",
                                    {"FlatLifeLevel", "FlatLifeGear", "FlatLifeTree", "BaseLife"}));
    CHECK(kernel.createCollectionNode("Col_IncLife", {"IncLifeGear", "IncLifeTree"}));
    CHECK(kernel.createCollectionNode("Col_MoreLife", {"MoreLifeGear", "MoreLifeTree"}));

    CHECK(kernel.createFormulaNode("FinalStr", "<Col_FlatStr> * (1 + <Col_IncStr> / 100)"));
    CHECK(kernel.createFormulaNode("FlatLifeStr", "<FinalStr> * <LifePerStr>"));
    CHECK(kernel.createFormulaNode(
        "FinalLife",
        "(<Col_FlatLife> + <FlatLifeStr>) * (1 + <Col_IncLife> / 100) * (1 + <Col_MoreLife> / 100)"));

    CHECK(kernel.evaluate());

    const auto finalLife = kernel.getNodeValue("FinalLife");
    REQUIRE(finalLife);

    constexpr double Col_FlatStr = 50.0 + 25.0;
    constexpr double Col_IncStr = 30.0 + 20.0;
    constexpr double Col_FlatLife = 100.0 + 40.0 + 20.0 + 10.0;
    constexpr double Col_IncLife = 10.0 + 15.0;
    constexpr double Col_MoreLife = 20.0 + 10.0;
    constexpr double FinalStr = Col_FlatStr * (1.0 + Col_IncStr / 100.0);
    constexpr double FlatLifeStr = FinalStr * 1.5;
    constexpr double expectedFinalLife =
        (Col_FlatLife + FlatLifeStr) * (1.0 + Col_IncLife / 100.0) * (1.0 + Col_MoreLife / 100.0);

    CHECK_EQ(*finalLife, doctest::Approx(expectedFinalLife).epsilon(0.001));
}

TEST_CASE("collection node operations") {
    StatKernel kernel;

    CHECK(kernel.createValueNode("a", 1));
    CHECK(kernel.createValueNode("b", 2));
    CHECK(kernel.createValueNode("c", 10));

    CHECK(kernel.createCollectionNode("sum", {"a", "b", "c"}, SF_COLLECTION_OP_SUM));
    CHECK(kernel.createCollectionNode("product", {"a", "b", "c"}, SF_COLLECTION_OP_PRODUCT));
    CHECK(kernel.createCollectionNode("median", {"a", "b", "c"}, SF_COLLECTION_OP_MEDIAN));
    CHECK(kernel.createCollectionNode("average", {"a", "b", "c"}, SF_COLLECTION_OP_AVERAGE));
    CHECK(kernel.createCollectionNode("min", {"a", "b", "c"}, SF_COLLECTION_OP_MIN));
    CHECK(kernel.createCollectionNode("max", {"a", "b", "c"}, SF_COLLECTION_OP_MAX));
    CHECK(kernel.createCollectionNode("count", {"a", "b", "c"}, SF_COLLECTION_OP_COUNT));

    CHECK(kernel.evaluate());

    checkValue(kernel, "sum", 13);
    checkValue(kernel, "product", 20);
    checkValue(kernel, "median", 2);
    checkValue(kernel, "average", doctest::Approx(13.0 / 3.0));
    checkValue(kernel, "min", 1);
    checkValue(kernel, "max", 10);
    checkValue(kernel, "count", 3);
}

TEST_CASE("collection node edge cases") {
    StatKernel kernel;

    CHECK(kernel.createValueNode("a", 1));
    CHECK(kernel.createValueNode("b", 2));
    CHECK(kernel.createValueNode("c", 10));
    CHECK(kernel.createValueNode("d", 100));

    SUBCASE("median with even dependency count averages the middle pair") {
        CHECK(kernel.createCollectionNode("median", {"a", "b", "c", "d"}, SF_COLLECTION_OP_MEDIAN));
        checkValue(kernel, "median", doctest::Approx(6.0));
    }

    SUBCASE("changing dependencies updates collection semantics") {
        CHECK(kernel.createCollectionNode("average", {"a", "b", "c"}, SF_COLLECTION_OP_AVERAGE));
        checkValue(kernel, "average", doctest::Approx(13.0 / 3.0));

        CHECK(kernel.setNodeDependencies("average", {"a", "d"}));
        checkValue(kernel, "average", doctest::Approx(50.5));
    }

    SUBCASE("duplicate dependencies are rejected on creation") {
        checkErrorCode(kernel.createCollectionNode("dup", {"a", "a"}, SF_COLLECTION_OP_SUM),
                       SF_ERR_DUPLICATE_DEPENDENCY);
    }

    SUBCASE("duplicate dependencies are rejected when rewiring") {
        CHECK(kernel.createCollectionNode("sum", {"a", "b"}, SF_COLLECTION_OP_SUM));
        checkErrorCode(kernel.setNodeDependencies("sum", {"c", "c"}), SF_ERR_DUPLICATE_DEPENDENCY);
    }

    SUBCASE("collection allows dependency removal and updates value") {
        CHECK(kernel.createCollectionNode("sum", {"a", "b"}, SF_COLLECTION_OP_SUM));
        checkValue(kernel, "sum", 3);

        CHECK(kernel.removeNode("a"));
        checkValue(kernel, "sum", 2);
    }
}

TEST_CASE("formula dependency removal is rejected") {
    StatKernel kernel;

    CHECK(kernel.createValueNode("value1", 5));
    CHECK(kernel.createFormulaNode("formula1", "<value1> * 2"));

    checkErrorCode(kernel.removeNode("value1"), SF_ERR_DEPENDENT_FORMULA_NODE);
    checkValue(kernel, "formula1", 10);
}

TEST_CASE("engine dependency string parsing") {
    Engine engine;

    CHECK_EQ(engine.createValueNode("a", 1), SF_OK);
    CHECK_EQ(engine.createValueNode("b", 2), SF_OK);
    CHECK_EQ(engine.createValueNode("c", 10), SF_OK);
    CHECK_EQ(engine.createCollectionNode("sum", SF_COLLECTION_OP_SUM), SF_OK);

    SUBCASE("commas and whitespace are both accepted") {
        CHECK_EQ(engine.setNodeDependency("sum", "a, b   c"), SF_OK);
        double value{};
        CHECK_EQ(engine.getNodeValue("sum", value), SF_OK);
        CHECK_EQ(value, 13);
    }

    SUBCASE("duplicate names in dependency string are rejected") {
        CHECK_EQ(engine.setNodeDependency("sum", "a a"), SF_ERR_DUPLICATE_DEPENDENCY);
    }
}

TEST_CASE("node creation errors") {
    StatKernel kernel;

    SUBCASE("duplicate node") {
        CHECK(kernel.createValueNode("val", 2));
        CHECK(kernel.createFormulaNode("formula", "<val> + 2"));
        CHECK(kernel.createCollectionNode("collection", {"formula"}));

        auto collectionVal = kernel.getNodeValue("collection");
        REQUIRE(collectionVal);
        CHECK_EQ(*collectionVal, 4);

        checkErrorCode(kernel.createValueNode("val", 5), SF_ERR_NODE_ALREADY_EXISTS);
        checkErrorCode(kernel.createFormulaNode("formula", "3 - <val>"),
                       SF_ERR_NODE_ALREADY_EXISTS);
        checkErrorCode(kernel.createCollectionNode("collection", {"val"}), SF_ERR_NODE_ALREADY_EXISTS);

        collectionVal = kernel.getNodeValue("collection");
        REQUIRE(collectionVal);
        CHECK_EQ(*collectionVal, 4);
    }
    SUBCASE("dependency doesn't exit") {
        checkErrorCode(kernel.createFormulaNode("formula", "3 - <sdfdfgdfgdfg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
        checkErrorCode(kernel.createCollectionNode("collection", {"fdshosadfojksdfoijksdrfff"}),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);

        checkErrorCode(kernel.getNodeValue("formula"), SF_ERR_NODE_NOT_FOUND);
        checkErrorCode(kernel.getNodeValue("collection"), SF_ERR_NODE_NOT_FOUND);
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(kernel.createFormulaNode("formula1", "3 - <<>"), SF_ERR_INVALID_DSL);
        checkErrorCode(kernel.getNodeValue("formula"), SF_ERR_NODE_NOT_FOUND);
    }
    SUBCASE("invalid collection operation") {
        checkErrorCode(kernel.createCollectionNode("collection",
                                                   {"val"},
                                                   static_cast<SF_CollectionOperation>(999)),
                       SF_ERR_INVALID_COLLECTION_OPERATION);
    }
}

TEST_CASE("node manipulation errors") {
    StatKernel kernel;
    CHECK(kernel.createValueNode("value1", 50));
    CHECK(kernel.createValueNode("value2", 60));
    CHECK(kernel.createFormulaNode("formula1", "<value1>"));
    CHECK(kernel.createFormulaNode("formula2", "<value2>"));
    CHECK(kernel.createCollectionNode("collection1", {"value1"}));
    CHECK(kernel.createCollectionNode("collection2", {"value2"}));

    SUBCASE("cyclic dependencies") {
        constexpr size_t distance = 100;

        CHECK(kernel.createFormulaNode("f0", "<value1>"));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(kernel.createFormulaNode("f" + std::to_string(i),
                                           "<f" + std::to_string(i - 1) + ">"));
        }
        checkErrorCode(kernel.setNodeFormula("f0", "<f" + std::to_string(distance - 1) + ">"),
                       SF_ERR_DEPENDENCY_LOOP);

        CHECK(kernel.createCollectionNode("c0", {"value1"}));
        for (size_t i = 1; i < distance; ++i) {
            CHECK(kernel.createCollectionNode("c" + std::to_string(i),
                                              {"c" + std::to_string(i - 1)}));
        }
        checkErrorCode(kernel.setNodeDependencies("c0", {"c" + std::to_string(distance - 1)}),
                       SF_ERR_DEPENDENCY_LOOP);
    }
    SUBCASE("self reference") {
        checkErrorCode(kernel.createFormulaNode("formula3", "<formula3>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(kernel.createCollectionNode("collection3", {"collection3"}), SF_ERR_SELF_REFERENCE);

        checkErrorCode(kernel.setNodeFormula("formula1", "<formula1>"), SF_ERR_SELF_REFERENCE);
        checkErrorCode(kernel.setNodeDependencies("collection1", {"collection1"}), SF_ERR_SELF_REFERENCE);
    }
    SUBCASE("wrong node types") {
        SUBCASE("setValue") {
            checkErrorCode(kernel.setNodeValue("formula1", 10), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "formula1", 50);

            checkErrorCode(kernel.setNodeValue("collection1", 10), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "collection1", 50);

            CHECK(kernel.setNodeValue("value1", 10));
            checkValue(kernel, "value1", 10);
        }
        SUBCASE("setFormula") {
            checkErrorCode(kernel.setNodeFormula("value1", "<value2>"), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "value1", 50);

            checkErrorCode(kernel.setNodeFormula("collection1", "<value2>"), SF_ERR_NODE_TYPE_MISMATCH);
            checkValue(kernel, "collection1", 50);

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

            CHECK(kernel.setNodeDependencies("collection1", {"value2"}));
            checkValue(kernel, "collection1", 60);
        }
    }
    SUBCASE("ill-formed dsl") {
        checkErrorCode(kernel.setNodeFormula("sdsflgjksdfjlksdfjlk", "1"), SF_ERR_NODE_NOT_FOUND);
        checkErrorCode(kernel.setNodeFormula("formula1", "<>324werwe534dsf"), SF_ERR_INVALID_DSL);
        checkErrorCode(kernel.setNodeFormula("formula1", "<dfdfsgggdfgdfsgdfsg>"),
                       SF_ERR_DEPENDENCY_DOESNT_EXIST);
    }
}
