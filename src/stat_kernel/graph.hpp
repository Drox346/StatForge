#pragma once

#include "error/internal/error.hpp"
#include "stat_kernel/node.hpp"
#include "types/definitions.hpp"

namespace statforge::statkernel {

class Graph {
public:
    [[nodiscard]] bool contains(NodeId const& id) const;

    // IMPPORTANT: For performance reasons calling "node()" on a non-existing node is
    // invalid and will terminate the program. Call contains() first if necessary.
    [[nodiscard]] Node& node(NodeId const& id);
    [[nodiscard]] Node const& node(NodeId const& id) const;

    [[nodiscard]] std::vector<NodeId> const& dependencies(NodeId const& id) const;
    [[nodiscard]] std::vector<NodeId> const& dependents(NodeId const& id) const;

    VoidResult addNode(NodeId id, Node node);
    VoidResult setNodeDependencies(NodeId id,
                                   std::vector<NodeId> deps,
                                   bool skipCycleCheck = false);
    VoidResult removeNode(NodeId const& id);

    void clear();

private:
    [[nodiscard]] std::vector<NodeId>& dependents(NodeId const& id);

    std::unordered_map<NodeId, Node> _nodes;
    std::unordered_map<NodeId, std::vector<NodeId>> _dependenciesMap;
    std::unordered_map<NodeId, std::vector<NodeId>> _dependentsMap;
};

} // namespace statforge::statkernel