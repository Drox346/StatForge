#include "../test_util.hpp"

#include "error/error.h"
#include "stat_kernel/stat_kernel.hpp"

#include <doctest/doctest.h>

using namespace statforge;

TEST_CASE("reset clears kernel state") {
    StatKernel kernel;

    CHECK(kernel.createValueNode("value1", 10));
    CHECK(kernel.createValueNode("value2", 5));
    CHECK(kernel.createFormulaNode("formula1", "<value1> + <value2>"));
    CHECK(kernel.createAggregatorNode("agg1", {"value1", "value2"}));
    CHECK(kernel.evaluate());

    checkValue(kernel, "formula1", 15);
    checkValue(kernel, "agg1", 15);

    kernel.reset();

    checkErrorCode(kernel.getNodeValue("value1"), SF_ERR_NODE_NOT_FOUND);
    checkErrorCode(kernel.getNodeValue("formula1"), SF_ERR_NODE_NOT_FOUND);
    checkErrorCode(kernel.getNodeValue("agg1"), SF_ERR_NODE_NOT_FOUND);

    CHECK(kernel.createValueNode("value1", 3));
    CHECK(kernel.createFormulaNode("formula1", "<value1> * 2"));
    CHECK(kernel.evaluate());
    checkValue(kernel, "formula1", 6);
}
