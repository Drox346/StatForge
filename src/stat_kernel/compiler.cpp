#include "compiler.hpp"
#include "error/internal/error.hpp"

#include "dsl/parser.hpp"
#include "stat_kernel/node.hpp"
#include "types/definitions.hpp"

#include <memory>
#include <string_view>

namespace {

constexpr bool skipCycleCheck = true;

}

namespace statforge::statkernel {

VoidResult Compiler::addCollectionNode(NodeId const& id, std::vector<NodeId> const& dependencies) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_graph.addNode(id, {}));

    // newly created nodes cannot appear as dependencies of existing nodes.
    // this guarantees skipCycleCheck is safe here.
    auto result = _graph.setNodeDependencies(id, dependencies, skipCycleCheck);
    if (!result) {
        _graph.removeNode(id);
        return result;
    }

    auto collect = [this, id]() -> NodeValue {
        NodeValue value{0};

        const auto& dependencies = _graph.dependencies(id);
        for (auto const& dependency : dependencies) {
            value += _graph.node(dependency).value;
        }

        return value;
    };

    _graph.node(
        id) = {.formula = collect, .value = 0, .type = NodeType::Collection, .dirty = true};

    return {};
}

VoidResult Compiler::addFormulaNode(NodeId const& id, std::string_view formula) {
    SF_RETURN_ERROR_IF_UNEXPECTED(_graph.addNode(id, {}));
    auto& node = _graph.node(id);
    node = {.formula = {}, .value = 0.0, .type = NodeType::Formula, .dirty = true};

    auto astResult = compileAst(id, formula);
    if (!astResult) {
        _graph.removeNode(id);
        return std::unexpected(std::move(astResult).error());
    }

    // newly created nodes cannot appear as dependencies of existing nodes.
    // this guarantees skipCycleCheck is safe here.
    auto dependencyResult =
        setNodeDependencies(id, dsl::extractDependencies(astResult->expr), skipCycleCheck);
    if (!dependencyResult) {
        _graph.removeNode(id);
        return dependencyResult;
    }
    node.formula = compileNodeFormula(std::move(*astResult));

    return {};
}

VoidResult Compiler::addValueNode(NodeId const& id, double value) {
    return _graph.addNode(
        id,
        {.formula = nullptr, .value = value, .type = NodeType::Value, .dirty = false});
}

VoidResult Compiler::setNodeFormula(NodeId const& id, std::string_view formula) {
    SF_RETURN_UNEXPECTED_IF(!_graph.contains(id),
                            SF_ERR_NODE_NOT_FOUND,
                            std::format(R"(Trying to set formula of non-existing node "{}")", id));
    SF_RETURN_UNEXPECTED_IF(
        _graph.contains(id) && (_graph.node(id).type != NodeType::Formula),
        SF_ERR_NODE_TYPE_MISMATCH,
        std::format(R"(Trying to manually change formula of non formula node "{}")", id));

    auto astResult = compileAst(id, formula);
    SF_RETURN_ERROR_IF_UNEXPECTED(astResult);

    auto dependencyResult = setNodeDependencies(id, dsl::extractDependencies(astResult->expr));
    SF_RETURN_ERROR_IF_UNEXPECTED(dependencyResult);

    _graph.node(id).formula = compileNodeFormula(std::move(*astResult));

    return {};
}

VoidResult Compiler::setCollectionNodeDependencies(NodeId const& id,
                                                   std::vector<NodeId> const& dependencies,
                                                   bool skipCycleCheck) {
    SF_RETURN_UNEXPECTED_IF(
        _graph.contains(id) && (_graph.node(id).type != NodeType::Collection),
        SF_ERR_NODE_TYPE_MISMATCH,
        std::format(R"(Trying to manually change dependencies of non collection node "{}")", id));

    return setNodeDependencies(id, dependencies, skipCycleCheck);
}

VoidResult Compiler::setNodeDependencies(NodeId const& id,
                                         std::vector<NodeId> const& dependencies,
                                         bool skipCycleCheck) {
    SF_RETURN_UNEXPECTED_IF(
        !_graph.contains(id),
        SF_ERR_NODE_NOT_FOUND,
        std::format(R"(Trying to set dependencies of non-existing node "{}")", id));

    assert(_graph.node(id).type != NodeType::Value);

    return _graph.setNodeDependencies(id, dependencies, skipCycleCheck);
}

Compiler::CompiledAstResult Compiler::compileAst(NodeId const& id, std::string_view formula) {
    CompiledAst ast{};
    ast.source = std::make_unique<std::string>(formula);

    auto astResult = dsl::Tokenizer{*ast.source}
                         .tokenize()
                         .and_then([](auto const& tokens) { return dsl::Parser{tokens}.parse(); })
                         .transform_error([&id](auto&& error) {
                             error.message.insert(0, std::format(R"(Node "{}": )", id));
                             return std::move(error);
                         });
    SF_RETURN_ERROR_IF_UNEXPECTED(astResult);

    ast.expr = std::move(*astResult.value());
    return ast;
}

NodeFormula Compiler::compileNodeFormula(CompiledAst compiledAst) {
    return [this, ast = std::make_shared<CompiledAst>(std::move(compiledAst))]() -> NodeValue {
        dsl::Context ctx{.nodeLookup = [this](std::string_view id) -> double {
            return _graph.node(std::string(id)).value;
        }};
        return dsl::evaluate(ast->expr, ctx);
    };
}

} // namespace statforge::statkernel
