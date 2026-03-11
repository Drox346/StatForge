#include "graph.hpp"

#include "error/error.h"
#include "error/internal/error.hpp"
#include "stat_kernel/node.hpp"
#include "types/definitions.hpp"

#include <algorithm>
#include <experimental/scope>
#include <stack>
#include <unordered_set>
#include <vector>

namespace statforge::statkernel {

namespace {

bool hasPath(NodeId const& src,
             NodeId const& target,
             std::unordered_map<NodeId, std::vector<NodeId>> const& dependencyMap) {
    //static for performance gains, avoiding heap allocations on every single call
    static std::stack<NodeId> work;
    static std::unordered_set<NodeId> visited;

    //clear data from previous calls
    while (!work.empty()) {
        work.pop();
    }
    visited.clear();

    work.push(src);

    while (!work.empty()) {
        auto current = work.top();
        work.pop();
        if (current == target) {
            return true;
        }
        if (!visited.insert(current).second) {
            continue;
        }

        auto it = dependencyMap.find(current);
        if (it == dependencyMap.end()) {
            continue;
        }
        for (auto const& dep : it->second) {
            work.push(dep);
        }
    }
    return false;
}

const std::vector<NodeId> emptyVec{};

} // namespace

bool Graph::contains(NodeId const& id) const {
    return _nodes.contains(id);
}

Node& Graph::node(NodeId const& id) {
    return const_cast<Node&>(static_cast<Graph const&>(*this).node(id));
}

Node const& Graph::node(NodeId const& id) const {
    auto it = _nodes.find(id);
    assert(it != _nodes.end());
    return it->second;
}

std::vector<NodeId> const& Graph::dependencies(NodeId const& id) const {
    auto it = _dependenciesMap.find(id);
    if (it != _dependenciesMap.end()) {
        return it->second;
    }

    return emptyVec;
}

std::vector<NodeId> const& Graph::dependents(NodeId const& id) const {
    auto it = _dependentsMap.find(id);
    if (it != _dependentsMap.end()) {
        return it->second;
    }

    return emptyVec;
}

VoidResult Graph::addNode(NodeId id, Node node) {
    auto [it, inserted] = _nodes.emplace(std::move(id), std::move(node));
    SF_RETURN_UNEXPECTED_IF(!inserted,
                            SF_ERR_NODE_ALREADY_EXISTS,
                            std::format(R"(Trying to add already existing node "{}")", it->first));

    return {};
}

VoidResult Graph::setNodeDependencies(NodeId id, std::vector<NodeId> newDeps, bool skipCycleCheck) {
    SF_RETURN_UNEXPECTED_IF(
        !contains(id),
        SF_ERR_NODE_NOT_FOUND,
        std::format(R"(Trying to set dependencies for non-existing node "{}")", id));

    for (auto const& dependency : newDeps) {
        SF_RETURN_UNEXPECTED_IF(
            !contains(dependency),
            SF_ERR_DEPENDENCY_DOESNT_EXIST,
            std::format(R"(Trying to add non-existing dependency "{}" to "{}")", dependency, id));
        SF_RETURN_UNEXPECTED_IF(dependency == id,
                                SF_ERR_SELF_REFERENCE,
                                std::format(R"("{}" is trying to set itself as dependency)", id));
        SF_RETURN_UNEXPECTED_IF(
            !skipCycleCheck && hasPath(dependency, id, _dependenciesMap),
            SF_ERR_DEPENDENCY_LOOP,
            //TODO better error msg to show cycle
            std::format(R"(Trying to set dependency of "{}" with cyclic dependency)", id));
    }

    auto& currentDeps = _dependenciesMap[id];

    // handle new dependencies
    for (const auto& dep : newDeps) {
        auto it = std::ranges::find(currentDeps, dep);
        if (it == currentDeps.end()) {
            _dependentsMap[dep].emplace_back(id);
        }
    }

    // handle removed dependencies
    for (const auto& previousDep : currentDeps) {
        auto it = std::ranges::find(newDeps, previousDep);
        if (it == newDeps.end()) {
            auto& dependentsPreviousDep = dependents(previousDep);
            auto itId = std::ranges::find(dependentsPreviousDep, id);
            if (itId != dependentsPreviousDep.end()) {
                dependentsPreviousDep.erase(itId);
            }
        }
    }

    currentDeps = std::move(newDeps);

    return {};
}

VoidResult Graph::removeNode(NodeId const& id) {
    auto nodeIt = _nodes.find(id);
    SF_RETURN_UNEXPECTED_IF(nodeIt == _nodes.end(),
                            SF_ERR_NODE_NOT_FOUND,
                            std::format(R"(Trying to remove non-existing node "{}")", id));

    // erase dependency from dependents
    auto dependentsIt = _dependentsMap.find(id);
    if (dependentsIt != _dependentsMap.end()) {
        // check that no dependent still needs this node
        for (auto const& dependentId : dependentsIt->second) {
            SF_RETURN_UNEXPECTED_IF(
                this->node(dependentId).type == NodeType::Formula,
                SF_ERR_DEPENDENT_FORMULA_NODE,
                std::format(R"(Trying to remove node "{}" that the formula node "{}" depends on)",
                            id,
                            dependentId));
        }

        // erase
        for (auto const& dependentId : dependentsIt->second) {
            auto depsIt = _dependenciesMap.find(dependentId);
            assert(depsIt != _dependenciesMap.end());

            auto& deps = depsIt->second;
            auto it = std::ranges::find(deps, id);
            if (it != deps.end()) {
                deps.erase(it);
            }
        }

        _dependentsMap.erase(dependentsIt);
    }

    // erase dependent from dependencies
    auto dependenciesIt = _dependenciesMap.find(id);
    if (dependenciesIt != _dependenciesMap.end()) {
        for (auto const& dependencyId : dependenciesIt->second) {
            auto depsIt = _dependentsMap.find(dependencyId);
            assert(depsIt != _dependentsMap.end());

            auto& dependents = depsIt->second;
            auto it = std::ranges::find(dependents, id);
            if (it != dependents.end()) {
                dependents.erase(it);
            }
        }

        _dependenciesMap.erase(dependenciesIt);
    }

    _nodes.erase(nodeIt);

    return {};
}

void Graph::clear() {
    _nodes.clear();
    _dependenciesMap.clear();
    _dependentsMap.clear();
}


// Will create an entry for "id" if no entry
// is found, making it safe to edit that entry.
// Some nodes may not have an entry due to not having dependents yet.
//
// no entry = no dependents (yet)
std::vector<NodeId>& Graph::dependents(NodeId const& id) {
    return _dependentsMap[id];
}

} // namespace statforge::statkernel