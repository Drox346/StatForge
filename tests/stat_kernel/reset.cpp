#include "../test_util.hpp"

#include "error/error.h"
#include "stat_kernel/stat_kernel.hpp"

#include <doctest/doctest.h>

using namespace statforge;

TEST_CASE("reset clears kernel state") {
    StatKernel kernel;

    CHECK(kernel.createValueCell("value1", 10));
    CHECK(kernel.createValueCell("value2", 5));
    CHECK(kernel.createFormulaCell("formula1", "<value1> + <value2>"));
    CHECK(kernel.createAggregatorCell("agg1", {"value1", "value2"}));
    CHECK(kernel.evaluate());

    checkValue(kernel, "formula1", 15);
    checkValue(kernel, "agg1", 15);

    kernel.reset();

    checkErrorCode(kernel.getCellValue("value1"), SF_ERR_CELL_NOT_FOUND);
    checkErrorCode(kernel.getCellValue("formula1"), SF_ERR_CELL_NOT_FOUND);
    checkErrorCode(kernel.getCellValue("agg1"), SF_ERR_CELL_NOT_FOUND);

    CHECK(kernel.createValueCell("value1", 3));
    CHECK(kernel.createFormulaCell("formula1", "<value1> * 2"));
    CHECK(kernel.evaluate());
    checkValue(kernel, "formula1", 6);
}
