#include "../test_util.hpp"
#include "api/cpp.hpp"

#include <doctest/doctest.h>
#include <print>
#include <string>

using namespace statforge;

#include <chrono>

TEST_CASE("benchmark node dependency evaluation") {
    constexpr size_t minNodeTarget = 100'000;

    Engine engine;
    CHECK_EQ(engine.createValueNode("root", 1), SF_OK);
    CHECK_EQ(engine.createFormulaNode("a0", "<root>"), SF_OK);

    size_t nextId = 1;     // next free "aN"
    size_t levelStart = 0; // first parent id in this level
    size_t levelCount = 1; // number of parents in this level

    bool ok = true;
    std::string errorMessage;

    auto t6 = std::chrono::steady_clock::now();

    while (nextId < minNodeTarget && ok) {
        for (size_t k = 0; k < levelCount && nextId < minNodeTarget; ++k) {
            const size_t parent = levelStart + k;
            const std::string formula = "root(2, <a" + std::to_string(parent) + ">) + 1";

            auto left = engine.createFormulaNode("a" + std::to_string(nextId), formula);
            if (left != SF_OK) {
                ok = false;
                errorMessage = engine.getLastError();
                break;
            }
            ++nextId;

            if (nextId >= minNodeTarget)
                break;

            auto right = engine.createFormulaNode("a" + std::to_string(nextId), formula);
            if (right != SF_OK) {
                ok = false;
                errorMessage = engine.getLastError();
                break;
            }
            ++nextId;
        }

        levelStart += levelCount;
        levelCount *= 2;
    }

    CHECK(ok);
    const size_t nodes = nextId;
    auto t7 = std::chrono::steady_clock::now();

    auto sweepAllNodes = [&]() {
        double value = 0;
        for (size_t i = 0; i < nodes; ++i) {
            if (engine.getNodeValue("a" + std::to_string(i), value) != SF_OK) {
                ok = false;
                errorMessage = engine.getLastError();
                break;
            }
        }
    };

    // initial full read
    auto t0 = std::chrono::steady_clock::now();
    sweepAllNodes();
    auto t1 = std::chrono::steady_clock::now();

    // dirty propagation
    auto t2 = std::chrono::steady_clock::now();
    if (engine.setNodeValue("root", 2) != SF_OK) {
        ok = false;
        errorMessage = engine.getLastError();
    }
    auto t3 = std::chrono::steady_clock::now();

    // full read of dirty nodes
    auto t4 = std::chrono::steady_clock::now();
    sweepAllNodes();
    auto t5 = std::chrono::steady_clock::now();

    auto ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    auto ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(t5 - t4).count();
    auto ms4 = std::chrono::duration_cast<std::chrono::milliseconds>(t7 - t6).count();
    std::print("Creation of {} formula nodes: {}ms\n"
               "Initial full read after creation: {}ms\n"
               "Setting {} nodes to dirty: {}ms\n"
               "Full read of {} dirty nodes: {}ms\n"
               "errors: {}\n",
               nodes,
               ms4,
               ms1,
               nodes,
               ms2,
               nodes,
               ms3,
               ok ? "none" : errorMessage);
}
