#include "stat_kernel.hpp"

#include "error/error.h"
#include "error/internal/error.hpp"
#include "stat_kernel/node.hpp"
#include "types/definitions.hpp"

#include <cassert>
#include <format>
#include <string_view>

namespace statforge {

using namespace statkernel;

StatKernel::StatKernel() : _compiler(_graph), _executor(_graph) {
}

VoidResult StatKernel::createCollectionNode(NodeId const& id,
                                            std::vector<NodeId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.addCollectionNode(id, dependencies));
    _executor.markAsDirtyLeaf(id);

    return {};
}

VoidResult StatKernel::createFormulaNode(NodeId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.addFormulaNode(id, formula));
    _executor.markAsDirtyLeaf(id);

    return {};
}

VoidResult StatKernel::createValueNode(NodeId const& id, double value) {
    return _compiler.addValueNode(id, value);
}

VoidResult StatKernel::removeNode(NodeId const& id) {
    if (auto result = _graph.removeNode(id); !result) [[unlikely]] {
        return result;
    }
    _executor.remove(id);

    return {};
}

VoidResult StatKernel::setNodeValue(NodeId const& id, NodeValue value) {
    SF_RETURN_UNEXPECTED_IF(!_graph.contains(id),
                            SF_ERR_NODE_NOT_FOUND,
                            std::format(R"(Trying to set value of non-existing node "{}")", id));

    auto& node = _graph.node(id);
    SF_RETURN_UNEXPECTED_IF(node.type != NodeType::Value,
                            SF_ERR_NODE_TYPE_MISMATCH,
                            std::format(R"(Trying to change value of non value node "{}")", id));

    if (node.value == value) {
        return {};
    }

    node.value = value;
    _executor.markDirty(id);
    return {};
}

VoidResult StatKernel::setNodeFormula(NodeId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.setNodeFormula(id, formula));
    _executor.markDirty(id);

    return {};
}

VoidResult StatKernel::setNodeDependencies(NodeId const& id,
                                           std::vector<NodeId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_compiler.setCollectionNodeDependencies(id, dependencies));
    _executor.markDirty(id);

    return {};
}

NodeValueResult StatKernel::getNodeValue(NodeId const& id) {
    return _executor.getNodeValue(id);
}

VoidResult StatKernel::evaluate() {
    return _executor.evaluate();
}

void StatKernel::reset() {
    _graph.clear();
    _executor.reset();
}

void StatKernel::setEvaluationType(statkernel::Executor::EvaluationType evaluationType) {
    _executor.setEvaluationType(evaluationType);
}

} // namespace statforge
