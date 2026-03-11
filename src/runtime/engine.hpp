#pragma once

#include "runtime/context.hpp"
#include "types/collection_operation.h"

#include <string>

namespace statforge::runtime {

class EngineImpl {
public:
    EngineImpl(statkernel::Executor::EvaluationType evaluationType =
                   statkernel::Executor::EvaluationType::Iterative);

    SF_ErrorCode createCollectionNode(NodeId const& name, SF_CollectionOperation operation);
    SF_ErrorCode createFormulaNode(NodeId const& name, std::string_view formula);
    SF_ErrorCode createValueNode(NodeId const& name, double value);
    SF_ErrorCode removeNode(NodeId const& name);
    SF_ErrorCode setNodeValue(NodeId const& name, double value);
    SF_ErrorCode setNodeFormula(NodeId const& name, std::string_view formula);
    SF_ErrorCode setNodeDependency(NodeId const& name, std::string_view dependencies);
    SF_ErrorCode getNodeValue(NodeId const& name, double& value);
    std::string getLastError();

    void evaluate();
    void reset();

private:
    Context ctx;
};

} // namespace statforge::runtime
