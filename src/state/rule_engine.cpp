#include <stdexcept>

#include "rule_engine.hpp"

namespace statforge {

void RuleEngine::addRule(const RuleId& id, const std::string& /*action*/, RuleValueType /*initValue*/) {
    if (_rules.contains(id)) {
        throw std::runtime_error("Rule already exists: " + id);
    }

    //_rules.emplace(id, Rule{Parser::parseAction(action), initValue});
}

void RuleEngine::editRule(const RuleId& id, const std::string& /*action*/, RuleValueType initValue) {
    auto rule = _rules.find(id);
    if (rule == _rules.end()) {
        throw std::runtime_error("Trying to edit non-existing rule: " + id);
    }

    //rule->second.action = Parser::parseAction(action);
    rule->second.previousValue = initValue;
}

void RuleEngine::removeRule(const RuleId& id) {
    _rules.erase(id);
}

void RuleEngine::reset() {
    _rules.clear();
}


} // namespace statforge