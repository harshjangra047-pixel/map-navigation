#ifndef SMARTROUTE_GRAPH_H
#define SMARTROUTE_GRAPH_H

#include "Node.h"
#include "Edge.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

/**
 * @brief Weighted Graph Representation using an Adjacency List.
 * 
 * Space Complexity: O(V + E)
 * Why Adjacency List?
 * - Real-world road networks are sparse graphs (average degree ~3-4 per intersection).
 * - An Adjacency Matrix would waste O(V^2) memory and make neighbor iteration O(V).
 * - Adjacency list allows neighbor inspection in O(deg(v)) time.
 */
class Graph {
private:
    std::unordered_map<int, Node> nodes;
    std::unordered_map<int, std::vector<Edge>> adj;
    int totalRoadCount; // Count of physical roads

public:
    Graph();

    // Graph Construction
    void addNode(const Node& node);
    void addEdge(int src, int dest, double distance, double baseTime, 
                 TrafficLevel traffic = TrafficLevel::LOW, bool bidirectional = true);

    // Queries
    bool hasNode(int id) const;
    const Node* getNode(int id) const;
    const Node* getNodeByName(const std::string& name) const;
    int findNodeIdByName(const std::string& name) const;
    const std::vector<Edge>& getNeighbors(int id) const;
    const std::unordered_map<int, Node>& getAllNodes() const;
    std::vector<Node> getNodesSortedById() const;

    // Edge manipulation
    bool setTrafficLevel(int src, int dest, TrafficLevel level, bool bidirectional = true);
    bool setBlocked(int src, int dest, bool blocked, bool bidirectional = true);
    const Edge* getEdge(int src, int dest) const;

    // Resetting state
    void resetTrafficAndBlockages();

    // Data Loaders
    bool loadFromFile(const std::string& filepath);
    void loadDefaultCampusMap();

    // Statistics
    size_t getNodeCount() const;
    size_t getDirectedEdgeCount() const;
    size_t getPhysicalRoadCount() const;
    int getBlockedRoadCount() const;
    int getHighTrafficRoadCount() const;
    std::vector<Edge> getAllUniqueEdges() const;
};

#endif // SMARTROUTE_GRAPH_H
