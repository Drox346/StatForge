#pragma once

#include "stat_kernel/node.hpp"
#include "types/definitions.hpp"

namespace statforge::debug {

void logNodes(std::string const& fileName,
              std::unordered_map<NodeId, statkernel::Node> const& nodes,
              std::unordered_map<NodeId, std::vector<NodeId>> const& dependencyMap);

} // namespace statforge::debug
