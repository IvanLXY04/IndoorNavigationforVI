#include "route_guidance.h"
#include <queue>
#include <set>
#include <map>

void RoutePlanner::addNode(const std::string& name) {
    if (graph.find(name) == graph.end())
        graph[name] = { name, {} };
}

void RoutePlanner::addEdge(const std::string& from, const std::string& to, int distance) {
    graph[from].neighbors.push_back({ to, distance });
    graph[to].neighbors.push_back({ from, distance });
}

std::vector<std::string> RoutePlanner::computeRoute(const std::string& start, const std::string& end) {
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> prev;
    std::set<std::pair<int, std::string>> pq;

    for (auto& kv : graph) {
        dist[kv.first] = INT_MAX;
    }

    dist[start] = 0;
    pq.insert({ 0, start });

    while (!pq.empty()) {
        const auto& top = *pq.begin();
        std::string u = top.second;
        pq.erase(pq.begin());

        if (u == end) break;

        for (const auto& neighbor : graph[u].neighbors) {
            const std::string& v = neighbor.first;
            int weight = neighbor.second;
            int newDist = dist[u] + weight;
            if (newDist < dist[v]) {
                pq.erase({ dist[v], v });
                dist[v] = newDist;
                prev[v] = u;
                pq.insert({ newDist, v });
            }
        }
    }

    std::vector<std::string> path;
    std::string at = end;
    while (prev.count(at)) {
        path.insert(path.begin(), at);
        at = prev[at];
    }
    if (path.empty() && start != end) return {}; // Return empty path if no route found
    path.insert(path.begin(), start);

    return path;
}