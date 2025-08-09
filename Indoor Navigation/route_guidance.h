#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct Node {
    std::string name;
    std::vector<std::pair<std::string, int>> neighbors;
};

class RoutePlanner {
public:
    void addNode(const std::string& name);
    void addEdge(const std::string& from, const std::string& to, int distance);
    std::vector<std::string> computeRoute(const std::string& start, const std::string& end);

private:
    std::unordered_map<std::string, Node> graph;
};