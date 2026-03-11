#pragma once

#include "error/internal/error.hpp"
#include "types/definitions.hpp"
#include "types/collection_operation.h"
#include <dsl/evaluator.hpp>
#include <stat_kernel/graph.hpp>
#include <stat_kernel/node.hpp>
#include <string_view>

namespace statforge::statkernel {

class Compiler {
public:
    Compiler() = delete;
    explicit Compiler(statkernel::Graph& graph) : _graph(graph) {
    }

    VoidResult addCollectionNode(NodeId const& id,
                                 std::vector<NodeId> const& dependencies,
                                 SF_CollectionOperation operation);
    VoidResult addFormulaNode(NodeId const& id, std::string_view formula);
    VoidResult addValueNode(NodeId const& id, double value);

    VoidResult setNodeFormula(NodeId const& id, std::string_view formula);
    VoidResult setCollectionNodeDependencies(NodeId const& id,
                                             std::vector<NodeId> const& dependencies,
                                             bool skipCycleCheck = false);

private:
    VoidResult setNodeDependencies(NodeId const& id,
                                   std::vector<NodeId> const& dependencies,
                                   bool skipCycleCheck = false);

    struct CompiledAst {
        // store "source" behind pointer to guarantee that copying or
        // moving this struct wont break string views referencing it
        std::unique_ptr<std::string> source;
        dsl::ExpressionTree expr;
    };
    using CompiledAstResult = Result<CompiledAst>;
    static CompiledAstResult compileAst(NodeId const& id, std::string_view formula);
    NodeFormula compileCollectionFormula(NodeId const& id, SF_CollectionOperation operation);
    NodeFormula compileNodeFormula(CompiledAst ast);

    statkernel::Graph& _graph;
};

} // namespace statforge::statkernel
