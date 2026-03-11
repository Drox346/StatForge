#include "executor.hpp"
#include "types/definitions.hpp"

#include <stack>

namespace statforge::statkernel {

void Executor::setEvaluationType(EvaluationType type) {
    if (type == EvaluationType::Recursive) {
        evaluateImpl = &Executor::evaluateRecursive;
    } else {
        evaluateImpl = &Executor::evaluateIterative;
    }
}

void Executor::reset() {
    _dirtyLeaves.clear();
}

void Executor::markDirty(NodeId const& id) {
    std::stack<NodeId> work;
    work.push(id);

    while (!work.empty()) {
        auto currentId = work.top();
        work.pop();
        auto& currentNode = _graph.node(currentId);
        const bool hasFormula = currentNode.type != NodeType::Value;

        if (currentNode.dirty) {
            continue;
        }

        currentNode.dirty = hasFormula;

        const auto& dependents = static_cast<const Graph&>(_graph).dependents(currentId);
        for (auto const& dependent : dependents) {
            work.push(dependent);
        }
        if (dependents.empty() && hasFormula) {
            _dirtyLeaves.emplace_back(currentId);
        }
    }
}

void Executor::markAsDirtyLeaf(NodeId const& id) {
    _dirtyLeaves.emplace_back(id);
}

void Executor::remove(NodeId const& id) {
    auto it = std::ranges::find(_dirtyLeaves, id);
    if (it != _dirtyLeaves.end()) {
        _dirtyLeaves.erase(it);
    }
}

void Executor::evaluate(NodeId const& id) {
    (this->*evaluateImpl)(id);
}

NodeValueResult Executor::getNodeValue(NodeId const& id) {
    SF_RETURN_UNEXPECTED_IF(!_graph.contains(id),
                            SF_ERR_NODE_NOT_FOUND,
                            std::format(R"(Trying to set value of non-existing node "{}")", id));

    auto& node = _graph.node(id);
    if (node.dirty) {
        evaluate(id);
    }
    return node.value;
}

VoidResult Executor::evaluate() {
    for (auto const& node : _dirtyLeaves) {
        evaluate(node);
    }
    _dirtyLeaves.clear();
    return {};
}

void Executor::evaluateRecursive(NodeId const& id) {
    auto& node = _graph.node(id);
    if (!node.dirty) {
        return;
    }

    for (auto const& dependency : _graph.dependencies(id)) {
        evaluateRecursive(dependency);
    }

    if (node.type != NodeType::Value) {
        node.value = node.formula();
    }
    node.dirty = false;
}

void Executor::evaluateIterative(NodeId const& id) {
    if (!_graph.node(id).dirty) {
        return;
    }

    enum class VisitState : uint8_t { Unvisited, Visiting, Visited };
    std::unordered_map<NodeId, VisitState> visitState;

    std::stack<NodeId> stack;
    stack.push(id);

    while (!stack.empty()) {
        auto currentId = stack.top();
        auto& state = visitState[currentId];

        if (state == VisitState::Visited) {
            stack.pop();
            continue;
        }

        auto& node = _graph.node(currentId);

        if (state == VisitState::Unvisited) {
            state = VisitState::Visiting;

            if (!node.dirty) {
                stack.pop();
                state = VisitState::Visited;
                continue;
            }

            for (const auto& dep : static_cast<const Graph&>(_graph).dependencies(currentId)) {
                if (visitState[dep] != VisitState::Visited) {
                    stack.push(dep);
                }
            }
        } else if (state == VisitState::Visiting) {
            if (node.formula) {
                node.value = node.formula();
            }
            node.dirty = false;
            state = VisitState::Visited;
            stack.pop();
        }
    }
}

} // namespace statforge::statkernel