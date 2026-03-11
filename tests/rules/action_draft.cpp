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
    statforge::NodeId target;
};
struct AddVal {
    statforge::NodeId target;
    statforge::NodeValue value;
};
struct AddForm {
    statforge::NodeId target;
    std::string formula;
};
struct AddAgg {
    statforge::NodeId target;
    std::string deps;
};
struct SetVal {
    statforge::NodeId target;
    statforge::NodeValue value;
};
struct SetForm {
    statforge::NodeId target;
    std::string formula;
};
struct SetDeps {
    statforge::NodeId target;
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
                       return kernel.createValueNode(action.target, action.value);
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
    // kernel.createValueNode(statforge::NodeId{"life"}, 100);

    // Action addLife = AddVal{.target = statforge::NodeId{"life"}, .value = statforge::NodeValue{1000}};
    // auto ruleResult = applyRule(addLife, kernel);
    // if (!ruleResult) {
    //     std::print("{}\n", ruleResult.error().message);
    // }

    // auto valResult = kernel.getNodeValue(statforge::NodeId{"life"});
    // if (valResult) {
    //     std::print("{}\n", valResult.value());
    // } else {
    //     std::print("{}\n", valResult.error().message);
    // }
}
