#include "puml.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

static std::string hashId(statforge::CellId const& id) {
    size_t const h = std::hash<statforge::CellId>{}(id);
    return std::string{"id"} + std::to_string(h);
}

} // namespace

namespace statforge::debug {

using namespace statkernel;

void logCells(std::string const& fileName,
              std::unordered_map<CellId, Cell> const& cells,
              std::unordered_map<CellId, std::vector<CellId>> const& dependencyMap) {
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

    for (auto const& [id, cell] : cells) {
        std::ostringstream valStream;
        valStream << std::fixed << std::setprecision(2) << cell.value;
        std::string const cellValStr = valStream.str();
        std::string const cellIdHash = hashId(id);

        rectangleDescriptions << "rectangle \"<b>" << id << "</b>\\nValue: ";
        if (cell.dirty) {
            rectangleDescriptions << "DIRTY";
        } else {
            rectangleDescriptions << cellValStr;
        }
        rectangleDescriptions << "\" as " << cellIdHash;
        if (cell.dirty) {
            rectangleDescriptions << " <<Dirty>>";
        }
        if (cell.type == CellType::Value) {
            rectangleDescriptions << " <<Value>>";
        }
        if (cell.type == CellType::Formula) {
            rectangleDescriptions << " <<Formula>>";
        }
        if (cell.type == CellType::Aggregator) {
            rectangleDescriptions << " <<Agg>>";
        }
        rectangleDescriptions << "\n";

        if (!dependencyMap.contains(id)) {
            continue;
        }
        const auto& dependencies = dependencyMap.at(id);
        for (auto const& dep : dependencies) {
            std::string const depIdHash = hashId(dep);
            dependencyDescriptions << depIdHash << " --> " << cellIdHash << "\n";
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