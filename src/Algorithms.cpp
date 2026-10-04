#include "Algorithms.h"
#include <queue>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include <cmath>
#include <sstream>

double Algorithms::calculateHeuristic(const Node& current, const Node& destination, RouteMode mode) {
    // Coordinate scale: 1 grid unit = ~0.07 km (70 meters).
    // Scaling by 0.05 km per grid unit mathematically guarantees an admissible lower bound: h(n) <= h*(n).
    double gridDist = current.euclideanDistanceTo(destination);
    double lowerBoundKm = gridDist * 0.05;

    switch (mode) {
        case RouteMode::SHORTEST_DISTANCE:
            // Straight-line physical displacement lower bound in km (Admissible & Monotonic)
            return lowerBoundKm;

        case RouteMode::FASTEST_TIME:
        case RouteMode::AVOID_HIGH_TRAFFIC:
        case RouteMode::EMERGENCY:
            // Upper bound walking/jogging speed = 10 km/h (~0.167 km/min)
            // Time (min) = lowerBoundKm / 0.20 km/min guarantees safe lower bound
            return lowerBoundKm / 0.20;

        default:
            return lowerBoundKm;
    }
}

SearchResult Algorithms::buildResult(
    const Graph& graph, int src, int dest,
    const std::unordered_map<int, int>& parent,
    const std::string& algoName, RouteMode mode, bool heuristic,
    int nodesExplored, double execMicroseconds) 
{
    SearchResult result;
    result.algorithmName = algoName;
    result.mode = mode;
    result.heuristicUsed = heuristic;
    result.sourceId = src;
    result.destId = dest;
    result.nodesExplored = nodesExplored;
    result.executionTimeMicroseconds = execMicroseconds;

    // Check if dest was reached
    if (src != dest && parent.find(dest) == parent.end()) {
        result.found = false;
        return result;
    }

    // Reconstruct path backward from dest to src
    std::vector<int> reversePath;
    int curr = dest;
    reversePath.push_back(curr);

    while (curr != src) {
        auto it = parent.find(curr);
        if (it == parent.end()) {
            result.found = false;
            return result;
        }
        curr = it->second;
        reversePath.push_back(curr);
    }

    std::reverse(reversePath.begin(), reversePath.end());
    result.path = reversePath;
    result.found = true;

    // Calculate detailed segment breakdowns
    double totalDist = 0.0;
    double totalTime = 0.0;

    for (size_t i = 0; i + 1 < result.path.size(); ++i) {
        int u = result.path[i];
        int v = result.path[i + 1];

        const Edge* edge = graph.getEdge(u, v);
        const Node* nodeU = graph.getNode(u);
        const Node* nodeV = graph.getNode(v);

        if (edge && nodeU && nodeV) {
            RouteSegment seg;
            seg.fromId = u;
            seg.fromName = nodeU->name;
            seg.toId = v;
            seg.toName = nodeV->name;
            seg.distance = edge->distance;
            seg.travelTime = edge->getEffectiveTravelTime();
            seg.traffic = edge->trafficLevel;
            seg.blocked = edge->blocked;

            totalDist += seg.distance;
            totalTime += seg.travelTime;
            result.segments.push_back(seg);
        }
    }

    result.totalDistance = totalDist;
    result.totalEstimatedTime = totalTime;
    return result;
}

SearchResult Algorithms::dijkstra(const Graph& graph, int src, int dest, RouteMode mode) {
    auto startTime = std::chrono::high_resolution_clock::now();

    SearchResult emptyResult;
    emptyResult.algorithmName = "Dijkstra's Algorithm";
    emptyResult.mode = mode;
    emptyResult.heuristicUsed = false;
    emptyResult.sourceId = src;
    emptyResult.destId = dest;

    if (!graph.hasNode(src) || !graph.hasNode(dest)) {
        emptyResult.found = false;
        return emptyResult;
    }

    if (src == dest) {
        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::micro>(endTime - startTime).count();
        emptyResult.found = true;
        emptyResult.path = {src};
        emptyResult.nodesExplored = 1;
        emptyResult.executionTimeMicroseconds = elapsed;
        return emptyResult;
    }

    // Min-heap storing pairs of (cost, vertex_id)
    std::priority_queue<std::pair<double, int>,
                        std::vector<std::pair<double, int>>,
                        std::greater<std::pair<double, int>>> pq;

    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> parent;
    std::unordered_set<int> settled;
    int nodesExplored = 0;

    const auto& allNodes = graph.getAllNodes();
    for (const auto& pair : allNodes) {
        dist[pair.first] = std::numeric_limits<double>::infinity();
    }

    dist[src] = 0.0;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [currentCost, u] = pq.top();
        pq.pop();

        if (settled.find(u) != settled.end()) {
            continue;
        }

        settled.insert(u);
        nodesExplored++;

        // Target reached: early termination
        if (u == dest) {
            break;
        }

        for (const auto& edge : graph.getNeighbors(u)) {
            if (edge.blocked) continue; // Skip blocked roads

            int v = edge.dest;
            double edgeWeight = edge.getCost(mode);

            if (dist[u] + edgeWeight < dist[v]) {
                dist[v] = dist[u] + edgeWeight;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::micro>(endTime - startTime).count();

    return buildResult(graph, src, dest, parent, "Dijkstra's Algorithm", mode, false, nodesExplored, elapsed);
}

SearchResult Algorithms::aStar(const Graph& graph, int src, int dest, RouteMode mode) {
    auto startTime = std::chrono::high_resolution_clock::now();

    SearchResult emptyResult;
    emptyResult.algorithmName = "A* Search Algorithm";
    emptyResult.mode = mode;
    emptyResult.heuristicUsed = true;
    emptyResult.sourceId = src;
    emptyResult.destId = dest;

    if (!graph.hasNode(src) || !graph.hasNode(dest)) {
        emptyResult.found = false;
        return emptyResult;
    }

    if (src == dest) {
        auto endTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::micro>(endTime - startTime).count();
        emptyResult.found = true;
        emptyResult.path = {src};
        emptyResult.nodesExplored = 1;
        emptyResult.executionTimeMicroseconds = elapsed;
        return emptyResult;
    }

    const Node* destNode = graph.getNode(dest);
    const Node* srcNode = graph.getNode(src);
    if (!destNode || !srcNode) {
        emptyResult.found = false;
        return emptyResult;
    }

    // Min-heap storing pairs of (f_score, vertex_id)
    std::priority_queue<std::pair<double, int>,
                        std::vector<std::pair<double, int>>,
                        std::greater<std::pair<double, int>>> openSet;

    std::unordered_map<int, double> gScore;
    std::unordered_map<int, double> fScore;
    std::unordered_map<int, int> parent;
    std::unordered_set<int> closedSet;
    int nodesExplored = 0;

    const auto& allNodes = graph.getAllNodes();
    for (const auto& pair : allNodes) {
        gScore[pair.first] = std::numeric_limits<double>::infinity();
        fScore[pair.first] = std::numeric_limits<double>::infinity();
    }

    gScore[src] = 0.0;
    fScore[src] = calculateHeuristic(*srcNode, *destNode, mode);
    openSet.push({fScore[src], src});

    while (!openSet.empty()) {
        auto [currentF, u] = openSet.top();
        openSet.pop();

        if (closedSet.find(u) != closedSet.end()) {
            continue;
        }

        closedSet.insert(u);
        nodesExplored++;

        if (u == dest) {
            break;
        }

        const Node* uNode = graph.getNode(u);
        if (!uNode) continue;

        for (const auto& edge : graph.getNeighbors(u)) {
            if (edge.blocked) continue;

            int v = edge.dest;
            const Node* vNode = graph.getNode(v);
            if (!vNode) continue;

            double tentativeG = gScore[u] + edge.getCost(mode);

            if (tentativeG < gScore[v]) {
                parent[v] = u;
                gScore[v] = tentativeG;
                double h = calculateHeuristic(*vNode, *destNode, mode);
                fScore[v] = tentativeG + h;
                openSet.push({fScore[v], v});
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::micro>(endTime - startTime).count();

    return buildResult(graph, src, dest, parent, "A* Search Algorithm", mode, true, nodesExplored, elapsed);
}

std::vector<int> Algorithms::bfsTraversal(const Graph& graph, int startNode) {
    std::vector<int> traversal;
    if (!graph.hasNode(startNode)) return traversal;

    std::unordered_set<int> visited;
    std::queue<int> q;

    visited.insert(startNode);
    q.push(startNode);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        traversal.push_back(u);

        for (const auto& edge : graph.getNeighbors(u)) {
            if (!edge.blocked && visited.find(edge.dest) == visited.end()) {
                visited.insert(edge.dest);
                q.push(edge.dest);
            }
        }
    }

    return traversal;
}

std::vector<int> Algorithms::dfsTraversal(const Graph& graph, int startNode) {
    std::vector<int> traversal;
    if (!graph.hasNode(startNode)) return traversal;

    std::unordered_set<int> visited;
    std::vector<int> stack;

    stack.push_back(startNode);

    while (!stack.empty()) {
        int u = stack.back();
        stack.pop_back();

        if (visited.find(u) != visited.end()) {
            continue;
        }

        visited.insert(u);
        traversal.push_back(u);

        // Push neighbors in reverse order to explore in consistent order
        const auto& neighbors = graph.getNeighbors(u);
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
            if (!it->blocked && visited.find(it->dest) == visited.end()) {
                stack.push_back(it->dest);
            }
        }
    }

    return traversal;
}

std::pair<SearchResult, std::string> Algorithms::findAlternativeRoute(
    const Graph& graph, int src, int dest, 
    const SearchResult& bestRoute, RouteMode mode) 
{
    if (!bestRoute.found || bestRoute.path.size() <= 2) {
        return {SearchResult(), "No alternative route feasible: direct or single-hop link."};
    }

    // Graph copy for temporary manipulation
    Graph tempGraph = graph;
    SearchResult bestAlternative;
    double minAltCost = std::numeric_limits<double>::infinity();

    // Iterate through intermediate segments of the best route and temporarily block them
    for (size_t i = 0; i + 1 < bestRoute.path.size(); ++i) {
        int u = bestRoute.path[i];
        int v = bestRoute.path[i + 1];

        // Temporarily block the segment
        tempGraph.setBlocked(u, v, true, true);

        // Compute candidate alternative route
        SearchResult candidate = dijkstra(tempGraph, src, dest, mode);
        candidate.algorithmName = "Alternative Route (Corridor Bypass)";

        if (candidate.found && candidate.path != bestRoute.path) {
            double cost = (mode == RouteMode::SHORTEST_DISTANCE) ? candidate.totalDistance : candidate.totalEstimatedTime;
            if (cost < minAltCost) {
                minAltCost = cost;
                bestAlternative = candidate;
            }
        }

        // Unblock for next test
        tempGraph.setBlocked(u, v, false, true);
    }

    if (!bestAlternative.found) {
        return {SearchResult(), "No alternative corridor available without using blocked roads or isolated nodes."};
    }

    // Generate educational comparison explanation
    std::stringstream explanation;
    double distDiff = bestAlternative.totalDistance - bestRoute.totalDistance;
    double timeDiff = bestAlternative.totalEstimatedTime - bestRoute.totalEstimatedTime;

    explanation << "System selected the PRIMARY route because it is ";
    if (std::abs(timeDiff) > 0.05) {
        explanation << (timeDiff > 0 ? "" : "-") << std::abs(timeDiff) << " min faster";
    }
    if (std::abs(distDiff) > 0.05) {
        explanation << " and " << std::abs(distDiff) << " km shorter";
    }
    explanation << " than the alternative bypass corridor.";

    return {bestAlternative, explanation.str()};
}
