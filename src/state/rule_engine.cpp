#include <stdexcept>

#include "rule_engine.hpp"
#include "dsl/parser.hpp"

namespace statforge {

void RuleEngine::addRule(const RuleId& id, const std::string& action, RuleValueType initValue) {
    if (m_rules.contains(id)) {
        throw std::runtime_error("Rule already exists: " + id);
    }

    m_rules.emplace(id, Rule{Parser::parseAction(action), initValue});
}

void RuleEngine::editRule(const RuleId& id, const std::string& action, RuleValueType initValue) {
    auto rule = m_rules.find(id);
    if (rule == m_rules.end()) {
        throw std::runtime_error("Trying to edit non-existing rule: " + id);
    }

    rule->second.action = Parser::parseAction(action);
    rule->second.previousValue = initValue;
}

void RuleEngine::removeRule(const RuleId& id) {
    m_rules.erase(id);
}

void RuleEngine::reset() {
    m_rules.clear();
}


} // namespace statforge