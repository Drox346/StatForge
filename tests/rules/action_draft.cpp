#include "../test_util.hpp"
#include "error/internal/error.hpp"
#include "stat_kernel/stat_kernel.hpp"
#include "types/definitions.hpp"

#include <cstdint>
#include <doctest/doctest.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

struct Remove {
    statforge::CellId target;
};
struct AddVal {
    statforge::CellId target;
    statforge::CellValue value;
};
struct AddForm {
    statforge::CellId target;
    std::string formula;
};
struct AddAgg {
    statforge::CellId target;
    std::string deps;
};
struct SetVal {
    statforge::CellId target;
    statforge::CellValue value;
};
struct SetForm {
    statforge::CellId target;
    std::string formula;
};
struct SetDeps {
    statforge::CellId target;
    std::string deps;
};

using Action = std::variant<Remove, AddVal, AddForm, AddAgg, SetVal, SetForm, SetDeps>;

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

statforge::VoidResult applyRule(const Action& action, statforge::StatKernel& kernel) {
    return std::visit(
        overloaded{[&kernel](const Remove& action) { return statforge::VoidResult{}; },
                   [&kernel](const AddVal& action) {
                       return kernel.createValueCell(action.target, action.value);
                   },
                   [&kernel](const AddForm& action) { return statforge::VoidResult{}; },
                   [&kernel](const AddAgg& action) { return statforge::VoidResult{}; },
                   [&kernel](const SetVal& action) { return statforge::VoidResult{}; },
                   [&kernel](const SetForm& action) { return statforge::VoidResult{}; },
                   [&kernel](const SetDeps& action) { return statforge::VoidResult{}; }},
        action);
}


// class Rule {
// public:
// private:
// };

// class RuleSystem {
// public:
//     //true = success
//     bool createRule();
//     bool appendAction();

// private:
//     std::unordered_map<std::string, Rule> _rules;
//     std::unordered_map<std::string, double> _values;
// };
#include <iostream>

TEST_CASE("main") {
    // statforge::StatKernel kernel;
    // kernel.createValueCell(statforge::CellId{"life"}, 100);

    // Action addLife = AddVal{.target = statforge::CellId{"life"}, .value = statforge::CellValue{1000}};
    // auto ruleResult = applyRule(addLife, kernel);
    // if (!ruleResult) {
    //     std::cout << ruleResult.error().message << "\n";
    // }

    // auto valResult = kernel.getCellValue(statforge::CellId{"life"});
    // if (valResult) {
    //     std::cout << valResult.value() << "\n";
    // } else {
    //     std::cout << valResult.error().message << "\n";
    // }
}
