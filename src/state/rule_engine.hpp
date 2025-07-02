#pragma once

#include <string>
#include <unordered_map>

#include "state/rule.hpp"

namespace statforge {

class RuleEngine {
public:
    void addRule(const RuleId& id, const std::string& action, RuleValueType initValue = 0);
    void editRule(const RuleId& id, const std::string& action, RuleValueType initValue = 0);
    void removeRule(const RuleId& id);

    void reset();

private:
    std::unordered_map<std::string, Rule> _rules;
};

} // namespace statforge