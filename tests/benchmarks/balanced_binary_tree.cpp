#include "../test_util.hpp"
#include "stat_kernel/stat_kernel.hpp"

#include <doctest/doctest.h>
#include <string>

using namespace statforge;

#include <chrono>
#include <iostream>
#include <random>

TEST_CASE("benchmark cell dependency evaluation") {
    constexpr size_t minCellTarget = 100'000;

    StatKernel kernel;
    CHECK(kernel.createValueCell("root", 1));
    CHECK(kernel.createFormulaCell("a0", "<root>"));

    size_t nextId = 1;     // next free "aN"
    size_t levelStart = 0; // first parent id in this level
    size_t levelCount = 1; // number of parents in this level

    VoidResult result{};

    auto t6 = std::chrono::steady_clock::now();

    while (nextId < minCellTarget && result.has_value()) {
        for (size_t k = 0; k < levelCount && nextId < minCellTarget; ++k) {
            const size_t parent = levelStart + k;

            auto left = kernel.createFormulaCell("a" + std::to_string(nextId),
                                                 "root(2, <a" + std::to_string(parent) + ">) + 1");
            if (!left) {
                result = left;
                break;
            }
            ++nextId;

            if (nextId >= minCellTarget)
                break;

            auto right = kernel.createFormulaCell("a" + std::to_string(nextId),
                                                  "root(2, <a" + std::to_string(parent) + ">) + 1");
            if (!right) {
                result = right;
                break;
            }
            ++nextId;
        }

        levelStart += levelCount;
        levelCount *= 2;
    }

    CHECK(result);
    const size_t cells = nextId;
    auto t7 = std::chrono::steady_clock::now();

    // initial clear dirty
    auto t0 = std::chrono::steady_clock::now();
    kernel.evaluate();
    auto t1 = std::chrono::steady_clock::now();

    // dirty propagation
    auto t2 = std::chrono::steady_clock::now();
    kernel.setCellValue("root", 2);
    auto t3 = std::chrono::steady_clock::now();

    // update all cells
    auto t4 = std::chrono::steady_clock::now();
    kernel.evaluate();
    auto t5 = std::chrono::steady_clock::now();
    static std::random_device dev;
    static std::mt19937 rng(dev());

    auto ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    auto ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count();
    auto ms4 = std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6).count();
    std::cout << "Creation of " << cells << " formula cells: " << ms4 << "ms\n"
              << "Initial evaluate after creation: " << ms1 << "ms\n"
              << "Setting " << cells << " cells to dirty: " << ms2 << "ms\n"
              << "Evaluate " << cells << " dirty cells: " << ms3 << "ms\n"
              << "errors: " << (result.has_value() ? "none" : result.error().message) << "\n";
}