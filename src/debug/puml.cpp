#include "puml.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

static std::string hashId(statforge::NodeId const& id) {
    size_t const h = std::hash<statforge::NodeId>{}(id);
    return std::string{"id"} + std::to_string(h);
}

} // namespace

namespace statforge::debug {

using namespace statkernel;

void logNodes(std::string const& fileName,
              std::unordered_map<NodeId, Node> const& nodes,
              std::unordered_map<NodeId, std::vector<NodeId>> const& dependencyMap) {
    std::ostringstream debugText;
    debugText << "@startuml\n"
                 "hide stereotype\n"
                 "skinparam rectangle {\n"
                 "  BackgroundColor<<Dirty>> Red\n"
                 "  BorderColor<<Agg>> Black\n"
                 "  BorderColor<<Formula>> Green\n"
                 "  BorderColor<<Value>> Red\n"
                 "  FontColor<<Dirty>> White\n"
                 "  BorderThickness 2.5\n"
                 "}\n\n";

    std::ostringstream rectangleDescriptions;
    std::ostringstream dependencyDescriptions;

    for (auto const& [id, node] : nodes) {
        std::ostringstream valStream;
        valStream << std::fixed << std::setprecision(2) << node.value;
        std::string const nodeValStr = valStream.str();
        std::string const nodeIdHash = hashId(id);

        rectangleDescriptions << "rectangle \"<b>" << id << "</b>\\nValue: ";
        if (node.dirty) {
            rectangleDescriptions << "DIRTY";
        } else {
            rectangleDescriptions << nodeValStr;
        }
        rectangleDescriptions << "\" as " << nodeIdHash;
        if (node.dirty) {
            rectangleDescriptions << " <<Dirty>>";
        }
        if (node.type == NodeType::Value) {
            rectangleDescriptions << " <<Value>>";
        }
        if (node.type == NodeType::Formula) {
            rectangleDescriptions << " <<Formula>>";
        }
        if (node.type == NodeType::Collection) {
            rectangleDescriptions << " <<Collection>>";
        }
        rectangleDescriptions << "\n";

        if (!dependencyMap.contains(id)) {
            continue;
        }
        const auto& dependencies = dependencyMap.at(id);
        for (auto const& dep : dependencies) {
            std::string const depIdHash = hashId(dep);
            dependencyDescriptions << depIdHash << " --> " << nodeIdHash << "\n";
        }
    }

    debugText << rectangleDescriptions.str() << dependencyDescriptions.str() << "@enduml";

    std::ofstream file{fileName + ".puml"};
    if (!file) {
        return;
    }
    file << debugText.str();
}

} // namespace statforge::debug
