#include <doctest/doctest.h>

#include "api/cpp.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <string>

using clk = std::chrono::steady_clock;

static inline long long ms(auto d) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
}

TEST_CASE("bench: high leaf count with isolated dependency chains") {
    constexpr std::size_t chains = 1000;
    constexpr std::size_t depth = 25;

    statforge::Engine engine;

    const auto t_create0 = clk::now();

    for (std::size_t i = 0; i < chains; ++i) {
        const std::string v = "v" + std::to_string(i);

        CHECK_EQ(engine.createValueCell(v, 1.0), SF_OK);

        std::string prev = v;

        for (std::size_t d = 0; d < depth; ++d) {
            const std::string cur = "c" + std::to_string(i) + "_" + std::to_string(d);

            const std::string formula = "root(2, <" + prev + ">) + 1";

            CHECK_EQ(engine.createFormulaCell(cur, formula), SF_OK);

            prev = cur;
        }
    }

    const auto t_create1 = clk::now();

    auto readLeaf = [&](std::size_t chainIdx) -> bool {
        const std::string leaf = "c" + std::to_string(chainIdx) + "_" + std::to_string(depth - 1);
        double value{};
        return engine.getCellValue(leaf, value);
    };

    const auto t_eval0 = clk::now();
    for (std::size_t i = 0; i < chains; ++i) {
        CHECK(readLeaf(i));
    }
    const auto t_eval1 = clk::now();

    const std::string targetV = "v0";
    const auto t_set0 = clk::now();
    CHECK_EQ(engine.setCellValue(targetV, 2.0), SF_OK);
    const auto t_set1 = clk::now();

    const auto t_evalOne0 = clk::now();
    CHECK(readLeaf(0));
    const auto t_evalOne1 = clk::now();

    std::mt19937_64 rng(0xC0FFEEULL);
    std::uniform_real_distribution<double> dist(-1000.0, 1000.0);

    const auto t_loop0 = clk::now();
    for (int it = 0; it < 2000; ++it) {
        const double x = dist(rng);
        CHECK_EQ(engine.setCellValue(targetV, x), SF_OK);
        CHECK(readLeaf(0));
    }
    const auto t_loop1 = clk::now();

    std::cout << "chains " << chains << " | depth " << depth << '\n'
              << "cell creation: " << ms(t_create1 - t_create0) << "ms\n"
              << "initial full leaf read: " << ms(t_eval1 - t_eval0) << "ms\n"
              << "setting value of a single value cell: " << ms(t_set1 - t_set0) << "ms\n"
              << "read of single chain leaf: " << ms(t_evalOne1 - t_evalOne0) << "ms\n"
              << "2000x random val + read of single chain leaf: " << ms(t_loop1 - t_loop0)
              << "ms\n";
}
