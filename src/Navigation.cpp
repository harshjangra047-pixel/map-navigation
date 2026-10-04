#include "Navigation.h"
#include <iostream>

Navigation::Navigation() : currentMode(RouteMode::FASTEST_TIME) {
    initialize();
}

bool Navigation::initialize(const std::string& dataPath) {
    if (!dataPath.empty() && graph.loadFromFile(dataPath)) {
        return true;
    }
    // Try relative data path
    if (graph.loadFromFile("data/city_map.txt")) {
        return true;
    }
    // Fall back to robust in-memory default
    graph.loadDefaultCampusMap();
    return true;
}

void Navigation::resetToDefaults() {
    graph.resetTrafficAndBlockages();
}

std::string Navigation::modeToString(RouteMode mode) {
    switch (mode) {
        case RouteMode::SHORTEST_DISTANCE:  return "Shortest Distance (Physical km)";
        case RouteMode::FASTEST_TIME:       return "Fastest Time (Traffic-weighted min)";
        case RouteMode::AVOID_HIGH_TRAFFIC: return "Avoid High Traffic (Smooth Arterials)";
        case RouteMode::EMERGENCY:          return "Emergency Priority (Express Transit)";
        default:                            return "Standard";
    }
}

SearchResult Navigation::findRoute(int src, int dest, bool useAStar) {
    if (useAStar) {
        return Algorithms::aStar(graph, src, dest, currentMode);
    } else {
        return Algorithms::dijkstra(graph, src, dest, currentMode);
    }
}

std::pair<SearchResult, SearchResult> Navigation::compareDijkstraVsAStar(int src, int dest) {
    SearchResult dijkstraRes = Algorithms::dijkstra(graph, src, dest, currentMode);
    SearchResult aStarRes = Algorithms::aStar(graph, src, dest, currentMode);
    return {dijkstraRes, aStarRes};
}

std::pair<SearchResult, std::string> Navigation::getAlternativeRoute(int src, int dest) {
    SearchResult best = Algorithms::dijkstra(graph, src, dest, currentMode);
    return Algorithms::findAlternativeRoute(graph, src, dest, best, currentMode);
}

bool Navigation::updateRoadTraffic(int src, int dest, TrafficLevel level) {
    return graph.setTrafficLevel(src, dest, level, true);
}

bool Navigation::setRoadBlockage(int src, int dest, bool blocked) {
    return graph.setBlocked(src, dest, blocked, true);
}

void Navigation::applyMorningRushHour() {
    // Heavy congestion on central core academic arterials
    graph.setTrafficLevel(6, 7, TrafficLevel::HIGH, true);  // Library <-> CSE Block
    graph.setTrafficLevel(7, 8, TrafficLevel::HIGH, true);  // CSE Block <-> IT & AI
    graph.setTrafficLevel(8, 9, TrafficLevel::HIGH, true);  // IT & AI <-> Engineering
    graph.setTrafficLevel(1, 2, TrafficLevel::MEDIUM, true); // Security <-> Admin
    graph.setTrafficLevel(4, 19, TrafficLevel::MEDIUM, true);// Parking <-> SAC
}

void Navigation::applyEmergencyCorridorSim(int blockedSrc, int blockedDest) {
    graph.setBlocked(blockedSrc, blockedDest, true, true);
}
