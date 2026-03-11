#include "api/cpp.hpp"

#include <print>

int main() {
    statforge::Engine engine;
    std::string lifeNodeName = "life";
    engine.createValueNode(lifeNodeName, 105);

    double nodeVal{};
    auto error = engine.getNodeValue(lifeNodeName, nodeVal);
    if (error) {
        std::print("Error: {}\n", engine.getLastError());
    } else {
        std::print("Val: {}\n", nodeVal);
    }

    auto creationError = engine.createValueNode(lifeNodeName, 100);
    std::print("Error: {}\n", engine.getLastError());

    return 0;
}