#ifndef SMARTROUTE_NAVIGATION_H
#define SMARTROUTE_NAVIGATION_H

#include "Graph.h"
#include "Algorithms.h"
#include <memory>
#include <string>

/**
 * @brief Navigation Service Engine coordinating graph models, algorithms,
 * dynamic traffic simulation, road blockage events, and route recalculation.
 */
class Navigation {
private:
    Graph graph;
    RouteMode currentMode;

public:
    Navigation();

    // Graph Initialization
    bool initialize(const std::string& dataPath = "");
    void resetToDefaults();

    // Accessors
    Graph& getGraph() { return graph; }
    const Graph& getGraph() const { return graph; }
    RouteMode getRouteMode() const { return currentMode; }
    void setRouteMode(RouteMode mode) { currentMode = mode; }
    static std::string modeToString(RouteMode mode);

    // Core Route Planning
    SearchResult findRoute(int src, int dest, bool useAStar = false);
    std::pair<SearchResult, SearchResult> compareDijkstraVsAStar(int src, int dest);
    std::pair<SearchResult, std::string> getAlternativeRoute(int src, int dest);

    // Dynamic Traffic & Blockage Management
    bool updateRoadTraffic(int src, int dest, TrafficLevel level);
    bool setRoadBlockage(int src, int dest, bool blocked);

    // Preset Traffic Scenarios for Impressive Live Demos
    void applyMorningRushHour();
    void applyEmergencyCorridorSim(int blockedSrc, int blockedDest);
};

#endif // SMARTROUTE_NAVIGATION_H
