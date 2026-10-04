#ifndef SMARTROUTE_ALGORITHMS_H
#define SMARTROUTE_ALGORITHMS_H

#include "Graph.h"
#include <vector>
#include <string>
#include <chrono>

/**
 * @brief Detailed breakdown of each leg/segment of a calculated route.
 */
struct RouteSegment {
    int fromId;
    std::string fromName;
    int toId;
    std::string toName;
    double distance;
    double travelTime;
    TrafficLevel traffic;
    bool blocked;
};

/**
 * @brief Comprehensive results container for graph search algorithms.
 * Used for comparing Dijkstra vs A*, rendering routes, and viva explanations.
 */
struct SearchResult {
    bool found;
    std::string algorithmName;
    RouteMode mode;
    bool heuristicUsed;
    int sourceId;
    int destId;
    std::vector<int> path;                     // Sequence of node IDs from source to dest
    std::vector<RouteSegment> segments;        // Step-by-step navigation breakdown
    double totalDistance;                      // Sum of physical distance in km
    double totalEstimatedTime;                 // Sum of travel time in minutes (with traffic)
    int nodesExplored;                         // Count of settled vertices during search
    double executionTimeMicroseconds;          // Actual measured CPU time in microseconds

    SearchResult()
        : found(false), algorithmName(""), mode(RouteMode::FASTEST_TIME),
          heuristicUsed(false), sourceId(-1), destId(-1),
          totalDistance(0.0), totalEstimatedTime(0.0),
          nodesExplored(0), executionTimeMicroseconds(0.0) {}
};

/**
 * @brief Core Data Structures & Algorithms implementations.
 * All algorithms are implemented manually from first principles using standard C++ STL.
 */
class Algorithms {
public:
    /**
     * @brief Dijkstra's Single-Source Shortest Path Algorithm.
     * 
     * DSA Concepts: Greedy Paradigm, Min-Heap Priority Queue.
     * Time Complexity: O((V + E) log V) with std::priority_queue.
     * Space Complexity: O(V) for distance table and parent pointers.
     * 
     * @param graph Road network graph.
     * @param src Source node ID.
     * @param dest Destination node ID.
     * @param mode Cost optimization mode (Shortest Distance / Fastest Time / etc.)
     * @return SearchResult containing optimal route and execution metrics.
     */
    static SearchResult dijkstra(const Graph& graph, int src, int dest, 
                                 RouteMode mode = RouteMode::FASTEST_TIME);

    /**
     * @brief A* Goal-Directed Heuristic Search Algorithm.
     * 
     * DSA Concepts: Informed Search, Euclidean Distance Heuristic, Min-Heap.
     * Evaluation Function: f(n) = g(n) + h(n)
     * - g(n): Exact cost accumulated from source to node n.
     * - h(n): Admissible Euclidean distance heuristic from node n to destination.
     * 
     * Time Complexity: O(E) in best case (directed straight to goal), up to O((V + E) log V) in worst case.
     * Space Complexity: O(V)
     * 
     * @param graph Road network graph.
     * @param src Source node ID.
     * @param dest Destination node ID.
     * @param mode Cost optimization mode.
     * @return SearchResult containing optimal route and execution metrics.
     */
    static SearchResult aStar(const Graph& graph, int src, int dest, 
                              RouteMode mode = RouteMode::FASTEST_TIME);

    /**
     * @brief Breadth-First Search (BFS) Traversal.
     * 
     * DSA Concepts: Queue (FIFO), Unweighted Shortest Path (minimum hops).
     * Time Complexity: O(V + E)
     * Space Complexity: O(V)
     * 
     * @param graph Road network graph.
     * @param startNode Starting node ID.
     * @return Traversal order of node IDs.
     */
    static std::vector<int> bfsTraversal(const Graph& graph, int startNode);

    /**
     * @brief Depth-First Search (DFS) Traversal.
     * 
     * DSA Concepts: Stack (LIFO / Recursion), Graph Exploration, Connected Components.
     * Time Complexity: O(V + E)
     * Space Complexity: O(V)
     * 
     * @param graph Road network graph.
     * @param startNode Starting node ID.
     * @return Traversal order of node IDs.
     */
    static std::vector<int> dfsTraversal(const Graph& graph, int startNode);

    /**
     * @brief Alternative Route Finder.
     * 
     * Calculates an alternative viable corridor by temporarily penalizing the primary
     * corridor's critical segments to avoid redundant overlap.
     * 
     * @param graph Road network graph.
     * @param src Source node ID.
     * @param dest Destination node ID.
     * @param bestRoute The previously computed optimal route.
     * @param mode Cost optimization mode.
     * @return Alternative SearchResult and an explanation string.
     */
    static std::pair<SearchResult, std::string> findAlternativeRoute(
        const Graph& graph, int src, int dest, 
        const SearchResult& bestRoute, RouteMode mode = RouteMode::FASTEST_TIME);

private:
    // Helper to assemble SearchResult from parent map
    static SearchResult buildResult(
        const Graph& graph, int src, int dest, 
        const std::unordered_map<int, int>& parent,
        const std::string& algoName, RouteMode mode, bool heuristic,
        int nodesExplored, double execMicroseconds);

    // Heuristic calculator for A*
    static double calculateHeuristic(const Node& current, const Node& destination, RouteMode mode);
};

#endif // SMARTROUTE_ALGORITHMS_H
