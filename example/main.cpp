#include "api/cpp.hpp"

#include <print>

int main() {
    statforge::Engine engine;
    std::string lifeCellName = "life";
    engine.createValueCell(lifeCellName, 105);

    double cellVal{};
    auto error = engine.getCellValue(lifeCellName, cellVal);
    if (error) {
        std::print("Error: {}\n", engine.getLastError());
    } else {
        std::print("Val: {}\n", cellVal);
    }

    auto creationError = engine.createValueCell(lifeCellName, 100);
    std::print("Error: {}\n", engine.getLastError());

    return 0;
}