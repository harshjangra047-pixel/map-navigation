#include "Graph.h"
#include "Algorithms.h"
#include "Navigation.h"
#include <iostream>
#include <cassert>
#include <cmath>

void runTests() {
    std::cout << "[TEST SUITE] Starting SmartRoute CGC Verification Tests...\n";

    Navigation nav;
    assert(nav.initialize("data/city_map.txt") && "CGC Campus Map initialization failed");
    const Graph& g = nav.getGraph();

    // Test 1: CGC Landran Node & Edge counts
    assert(g.getNodeCount() == 26 && "Expected 26 landmarks in CGC Landran dataset");
    assert(g.getPhysicalRoadCount() == 55 && "Expected 55 physical pathways");
    std::cout << "  [PASS] Test 1: CGC Landran dataset loaded with 26 landmarks and 55 pathways.\n";

    // Test 2: Source == Destination
    SearchResult resSame = Algorithms::dijkstra(g, 0, 0);
    assert(resSame.found && "Route to self should be found");
    assert(resSame.path.size() == 1 && resSame.path[0] == 0);
    assert(resSame.totalDistance == 0.0 && "Distance to self must be 0");
    assert(resSame.totalEstimatedTime == 0.0 && "Time to self must be 0");
    std::cout << "  [PASS] Test 2: Source == Destination edge case handled.\n";

    // Test 3: Dijkstra vs A* on clear campus map (Main Gate 1 [0] -> Girls Hostel [17])
    SearchResult dRes = Algorithms::dijkstra(g, 0, 17, RouteMode::SHORTEST_DISTANCE);
    SearchResult aRes = Algorithms::aStar(g, 0, 17, RouteMode::SHORTEST_DISTANCE);
    assert(dRes.found && aRes.found && "Path must be found between Main Gate 1 and Girls Hostel");
    assert(std::abs(dRes.totalDistance - aRes.totalDistance) < 1e-4 && "Dijkstra and A* must find identical optimal shortest distance");
    assert(aRes.nodesExplored <= dRes.nodesExplored && "A* should explore <= Dijkstra nodes");
    std::cout << "  [PASS] Test 3: Dijkstra & A* found optimal distance (" << dRes.totalDistance << " km).\n";
    std::cout << "         Dijkstra explored: " << dRes.nodesExplored << " | A* explored: " << aRes.nodesExplored << " nodes.\n";

    // Test 4: Dynamic Campus Traffic Simulation
    SearchResult baseTimeRoute = Algorithms::dijkstra(g, 0, 17, RouteMode::FASTEST_TIME);
    double initialTime = baseTimeRoute.totalEstimatedTime;
    std::cout << "         Base route path: ";
    for (int p : baseTimeRoute.path) std::cout << p << " ";
    std::cout << " (Initial time: " << initialTime << " min)\n";

    // Simulate heavy student rush hour along the base route
    for (size_t i = 0; i + 1 < baseTimeRoute.path.size(); ++i) {
        nav.updateRoadTraffic(baseTimeRoute.path[i], baseTimeRoute.path[i+1], TrafficLevel::HIGH);
    }
    SearchResult congestedTimeRoute = Algorithms::dijkstra(nav.getGraph(), 0, 17, RouteMode::FASTEST_TIME);
    std::cout << "         Congested route path: ";
    for (int p : congestedTimeRoute.path) std::cout << p << " ";
    std::cout << " (Congested time: " << congestedTimeRoute.totalEstimatedTime << " min)\n";
    
    // Path should divert or time should increase
    assert((congestedTimeRoute.path != baseTimeRoute.path || congestedTimeRoute.totalEstimatedTime > initialTime) 
           && "Traffic should affect route or travel time");
    std::cout << "  [PASS] Test 4: Dynamic traffic simulation alters route/travel time appropriately.\n";

    // Test 5: Dynamic Pathway Blockage & Re-routing
    nav.resetToDefaults();
    // Block direct pathway 2 <-> 6 (Admin Block <-> Central Lawn)
    nav.setRoadBlockage(2, 6, true);
    SearchResult blockRoute = Algorithms::dijkstra(nav.getGraph(), 0, 5, RouteMode::FASTEST_TIME);
    assert(blockRoute.found && "Alternative route must exist to Central Library around blocked road");
    // Ensure blocked edge is not in path
    for (size_t i = 0; i + 1 < blockRoute.path.size(); ++i) {
        bool usesBlocked = (blockRoute.path[i] == 2 && blockRoute.path[i+1] == 6) ||
                           (blockRoute.path[i] == 6 && blockRoute.path[i+1] == 2);
        assert(!usesBlocked && "Calculated path must NOT use blocked edge");
    }
    std::cout << "  [PASS] Test 5: Pathway blockage successfully bypassed via dynamic recalculation.\n";

    // Test 6: Complete Disconnection / Unreachable Destination
    Graph isolatedGraph = nav.getGraph();
    for (const auto& edge : isolatedGraph.getNeighbors(23)) {
        isolatedGraph.setBlocked(edge.src, edge.dest, true, true);
    }
    SearchResult unreachableRes = Algorithms::dijkstra(isolatedGraph, 0, 23);
    assert(!unreachableRes.found && "Route should be flagged as not found when completely isolated");
    std::cout << "  [PASS] Test 6: Unreachable / isolated destination handled cleanly without crash.\n";

    // Test 7: BFS & DFS traversals
    nav.resetToDefaults();
    auto bfsSeq = Algorithms::bfsTraversal(nav.getGraph(), 0);
    auto dfsSeq = Algorithms::dfsTraversal(nav.getGraph(), 0);
    assert(bfsSeq.size() == 26 && "BFS must visit all 26 connected landmarks");
    assert(dfsSeq.size() == 26 && "DFS must visit all 26 connected landmarks");
    assert(bfsSeq[0] == 0 && dfsSeq[0] == 0 && "Start node must be 0 (Main Gate 1)");
    std::cout << "  [PASS] Test 7: BFS and DFS traversed all 26 CGC Landran landmarks.\n";

    // Test 8: Alternative Route Finder
    SearchResult primary = Algorithms::dijkstra(nav.getGraph(), 0, 17);
    auto [altRoute, reason] = Algorithms::findAlternativeRoute(nav.getGraph(), 0, 17, primary);
    assert(altRoute.found && "Alternative corridor should be discovered");
    assert(altRoute.path != primary.path && "Alternative route must differ from primary");
    assert(!reason.empty() && "Explanation should be non-empty");
    std::cout << "  [PASS] Test 8: Alternative bypass route found successfully: " << reason << "\n";

    std::cout << "\n[ALL 8 CGC LANDRAN VERIFICATION TESTS PASSED WITH 100% SUCCESS!]\n";
}

int main() {
    runTests();
    return 0;
}
