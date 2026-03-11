#include "error/error.h"
#include "error/internal/error.hpp"
#include "stat_kernel/stat_kernel.hpp"
#include "types/definitions.hpp"

#include <doctest/doctest.h>

template <typename T>
void checkErrorCode(statforge::Result<T> result, SF_ErrorCode code) {
    REQUIRE_FALSE(result);
    CHECK_EQ(result.error().errorCode, code);
}

template <typename Expected>
inline void checkValue(statforge::StatKernel& kernel,
                       std::string const& nodeId,
                       Expected const& value) {
    auto result = kernel.getNodeValue(nodeId);
    REQUIRE(result);
    CHECK(*result == value);
}
