#include <doctest/doctest.h>

#include <chrono>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "stat_kernel/compiler.hpp"
#include "stat_kernel/executor.hpp"
#include "stat_kernel/stat_kernel.hpp"

using clk = std::chrono::steady_clock;

static inline long long ms(auto d) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
}

// Prevent “unused result” / overly clever elision in microbenchmarks
static volatile double g_sink = 0.0;

TEST_CASE("bench: high leaf count with isolated dependency chains") {
    constexpr std::size_t chains = 1000;
    constexpr std::size_t depth = 25;

    statforge::StatKernel kernel;
    kernel.setEvaluationType(statforge::statkernel::Executor::EvaluationType::Recursive);

    // --- Build -------------------------------------------------------------
    // Each chain i:
    //   value cell:  v<i>
    //   formula cells: c<i>_0 depends on v<i>, c<i>_1 depends on c<i>_0, ...
    //
    // Leaves are c<i>_(depth-1). Chains are isolated from each other.

    const auto t_create0 = clk::now();

    for (std::size_t i = 0; i < chains; ++i) {
        const std::string v = "v" + std::to_string(i);

        // API ADJUST: createValueCell(name, value)
        // If yours returns expected/result, wrap with CHECK(...)
        CHECK(kernel.createValueCell(v, 1.0));

        std::string prev = v;

        for (std::size_t d = 0; d < depth; ++d) {
            const std::string cur = "c" + std::to_string(i) + "_" + std::to_string(d);

            // Keep formula simple and stable.
            // If your DSL uses <name> for refs, this matches your earlier examples.
            const std::string formula = "root(<" + prev + ">, 2) + 1";

            CHECK(kernel.createFormulaCell(cur, formula));

            prev = cur;
        }
    }

    const auto t_create1 = clk::now();

    // --- Initial evaluation ------------------------------------------------
    const auto t_eval0 = clk::now();

    // API ADJUST: if you only have evaluate(), call that.
    // If you have evaluateAll(), use it. If you require evaluating via getValue, do that.
    CHECK(kernel.evaluate());

    const auto t_eval1 = clk::now();

    // --- Single value set + eval should be tiny ----------------------------
    // We’ll update one chain root and evaluate only that chain’s leaf.
    const std::string targetV = "v0";
    const std::string targetLeaf = "c0_" + std::to_string(depth - 1);

    const auto t_set0 = clk::now();

    // API ADJUST: setValueCell(name, value) / setCellValue / setValue
    CHECK(kernel.setCellValue(targetV, 2.0));

    const auto t_set1 = clk::now();

    const auto t_evalOne0 = clk::now();

    // API ADJUST: evaluate(cellName) or evaluate(name) should evaluate just that subtree/chain.
    CHECK(kernel.getCellValue(targetLeaf));

    const auto t_evalOne1 = clk::now();

    // --- 2000× random value + eval single chain ----------------------------
    std::mt19937_64 rng(0xC0FFEEULL);
    std::uniform_real_distribution<double> dist(-1000.0, 1000.0);

    const auto t_loop0 = clk::now();

    for (int it = 0; it < 2000; ++it) {
        const double x = dist(rng);
        CHECK(kernel.setCellValue(targetV, x));

        // API ADJUST: if you have getValue(name), read the leaf to enforce actual work.
        // Otherwise delete this block.
        auto val = kernel.getCellValue(targetLeaf); // expected<double, ErrorInfo> or similar?
        CHECK(val.has_value());
        g_sink += *val;
    }

    const auto t_loop1 = clk::now();

    // --- Report ------------------------------------------------------------
    MESSAGE("chains ", chains, " | depth ", depth);
    MESSAGE("cell creation: ", ms(t_create1 - t_create0), "ms");
    MESSAGE("initial eval: ", ms(t_eval1 - t_eval0), "ms");
    MESSAGE("setting value of a single value cell: ", ms(t_set1 - t_set0), "ms");
    MESSAGE("eval of single chain leaf: ", ms(t_evalOne1 - t_evalOne0), "ms");
    MESSAGE("2000x random val + eval of single chain: ", ms(t_loop1 - t_loop0), "ms");
}
