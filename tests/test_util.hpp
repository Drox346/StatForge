#include "error/error.h"
#include "error/internal/error.hpp"
#include "spreadsheet/spreadsheet.hpp"
#include "types/definitions.hpp"

#include <doctest/doctest.h>

template <typename T>
void checkErrorCode(statforge::Result<T> result, SF_ErrorCode code) {
    REQUIRE_FALSE(result);
    CHECK_EQ(result.error().errorCode, code);
}
