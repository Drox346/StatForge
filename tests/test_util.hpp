#include "common/error.h"
#include "common/internal/error.hpp"
#include <doctest/doctest.h>

template <typename T>
void checkErrorCode(statforge::Result<T> result, SF_ErrorCode code) {
    REQUIRE_FALSE(result);
    CHECK_EQ(result.error().errorCode, code);
}
