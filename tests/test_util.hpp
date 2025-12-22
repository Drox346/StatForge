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

inline void checkValue(statforge::Spreadsheet& sheet,
                       std::string const& cellId,
                       statforge::CellValue value) {
    auto result = sheet.getCellValue(cellId);
    REQUIRE(result);
    CHECK_EQ(*result, value);
}