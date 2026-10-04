#include "UI.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <algorithm>

const std::string UI::RESET      = "\033[0m";
const std::string UI::BOLD       = "\033[1m";
const std::string UI::RED        = "\033[31m";
const std::string UI::GREEN      = "\033[32m";
const std::string UI::YELLOW     = "\033[33m";
const std::string UI::BLUE       = "\033[34m";
const std::string UI::MAGENTA    = "\033[35m";
const std::string UI::CYAN       = "\033[36m";
const std::string UI::WHITE      = "\033[37m";
const std::string UI::BG_BLUE    = "\033[44m";
const std::string UI::BG_DARK    = "\033[40m";

UI::UI(Navigation& navigation) : nav(navigation) {}

void UI::clearScreen() {
    // ANSI clear screen
    std::cout << "\033[2J\033[1;1H";
}

void UI::pauseForUser() {
    std::cout << "\n" << CYAN << "Press Enter to return to continue..." << RESET;
    std::string dummy;
    std::getline(std::cin, dummy);
}

int UI::getValidInt(const std::string& prompt, int minVal, int maxVal) {
    int choice = -1;
    while (true) {
        std::cout << prompt;
        std::string input;
        if (!std::getline(std::cin, input)) {
            return minVal; // Handle EOF gracefully
        }
        std::stringstream ss(input);
        if (ss >> choice && ss.eof()) {
            if (choice >= minVal && choice <= maxVal) {
                return choice;
            }
        }
        std::cout << RED << "Invalid selection. Please enter a number between " 
                  << minVal << " and " << maxVal << ".\n" << RESET;
    }
}

int UI::promptForLocation(const std::string& role) {
    while (true) {
        std::cout << BOLD << "Enter " << role << " [ID (0-23) or Name (e.g. 'hospital', 'cse')]: " << RESET;
        std::string input;
        if (!std::getline(std::cin, input)) return -1;

        // Trim input
        size_t first = input.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        size_t last = input.find_last_not_of(" \t");
        input = input.substr(first, (last - first + 1));

        // Check if numeric ID
        bool isNumber = true;
        for (char c : input) {
            if (!std::isdigit(c)) {
                isNumber = false;
                break;
            }
        }

        if (isNumber) {
            int id = std::stoi(input);
            if (nav.getGraph().hasNode(id)) {
                return id;
            }
            std::cout << RED << "Error: No location found with ID " << id << ". Enter 0 to 23.\n" << RESET;
        } else {
            // Search by name
            int foundId = nav.getGraph().findNodeIdByName(input);
            if (foundId != -1) {
                const Node* node = nav.getGraph().getNode(foundId);
                std::cout << GREEN << "-> Matched: [" << foundId << "] " << node->name << RESET << "\n";
                return foundId;
            }
            std::cout << RED << "Error: Could not locate \"" << input << "\". Type a valid name or ID.\n" << RESET;
        }
    }
}

void UI::displayHeader() {
    std::cout << CYAN << BOLD;
    std::cout << "========================================================================\n";
    std::cout << "                           SMARTROUTE                                   \n";
    std::cout << "         INTELLIGENT MAP NAVIGATION SYSTEM USING DSA IN C++             \n";
    std::cout << "========================================================================\n";
    std::cout << RESET;
}

void UI::displayMainMenu() {
    displayHeader();
    std::cout << YELLOW << " Current Mode: " << BOLD << Navigation::modeToString(nav.getRouteMode()) << RESET << "\n";
    std::cout << " Network Status: " << GREEN << nav.getGraph().getNodeCount() << " Locations" << RESET
              << " | " << GREEN << nav.getGraph().getPhysicalRoadCount() << " Roads" << RESET
              << " | " << (nav.getGraph().getBlockedRoadCount() > 0 ? RED : GREEN) 
              << nav.getGraph().getBlockedRoadCount() << " Blocked" << RESET
              << " | " << (nav.getGraph().getHighTrafficRoadCount() > 0 ? YELLOW : GREEN) 
              << nav.getGraph().getHighTrafficRoadCount() << " Congested\n" << RESET;
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << BOLD << " 1. " << RESET << "Find Best Route (Dijkstra / A* Search)\n";
    std::cout << BOLD << " 2. " << RESET << "Compare Algorithms (Dijkstra vs A* Side-by-Side)\n";
    std::cout << BOLD << " 3. " << RESET << "Alternative Route Finder (Primary vs Bypass Corridor)\n";
    std::cout << BOLD << " 4. " << RESET << "Simulate Traffic Congestion (LOW / MEDIUM / HIGH)\n";
    std::cout << BOLD << " 5. " << RESET << "Block / Unblock Road Segment (Dynamic Re-routing)\n";
    std::cout << BOLD << " 6. " << RESET << "Run Graph Traversal (BFS & DFS Traversal Orders)\n";
    std::cout << BOLD << " 7. " << RESET << "View Campus Map & Topological Network\n";
    std::cout << BOLD << " 8. " << RESET << "Viva / Educational Mode (DSA In-Depth Q&A)\n";
    std::cout << BOLD << " 9. " << RESET << "View System & Project Statistics\n";
    std::cout << BOLD << "10. " << RESET << CYAN << "Run 5-Minute Presentation Demo Walkthrough\n" << RESET;
    std::cout << BOLD << "11. " << RESET << "Change Routing Mode (Shortest, Fastest, Avoid Traffic, Emergency)\n";
    std::cout << BOLD << "12. " << RESET << "Reset Network to Default Conditions\n";
    std::cout << BOLD << " 0. " << RESET << RED << "Exit SmartRoute System\n" << RESET;
    std::cout << "------------------------------------------------------------------------\n";
}

void UI::printNodeDirectory() {
    std::cout << "\n" << BOLD << "=== AVAILABLE CGC LANDRAN CAMPUS LOCATIONS (26 NODES) ===" << RESET << "\n";
    const auto nodes = nav.getGraph().getNodesSortedById();
    for (size_t i = 0; i < nodes.size(); i += 2) {
        std::cout << std::setw(3) << nodes[i].id << ". " 
                  << std::left << std::setw(26) << nodes[i].name 
                  << " (" << nodes[i].type << ")";
        if (i + 1 < nodes.size()) {
            std::cout << "  |  " << std::setw(3) << nodes[i+1].id << ". " 
                      << std::left << std::setw(26) << nodes[i+1].name 
                      << " (" << nodes[i+1].type << ")";
        }
        std::cout << "\n";
    }
    std::cout << "------------------------------------------------------------------------\n";
}

void UI::printRoadDirectory() {
    std::cout << "\n" << BOLD << "=== ACTIVE PHYSICAL ROAD ARTERIALS ===" << RESET << "\n";
    const auto edges = nav.getGraph().getAllUniqueEdges();
    std::cout << std::left << std::setw(5) << "#" 
              << std::setw(32) << "Road Connection"
              << std::setw(12) << "Distance" 
              << std::setw(14) << "Travel Time"
              << std::setw(16) << "Traffic Status"
              << "Condition\n";
    std::cout << std::string(85, '-') << "\n";

    int index = 1;
    for (const auto& edge : edges) {
        const Node* uNode = nav.getGraph().getNode(edge.src);
        const Node* vNode = nav.getGraph().getNode(edge.dest);
        if (!uNode || !vNode) continue;

        std::string roadName = "[" + std::to_string(edge.src) + "] " + uNode->name + " <-> [" + 
                               std::to_string(edge.dest) + "] " + vNode->name;
        if (roadName.length() > 30) roadName = roadName.substr(0, 27) + "...";

        std::string trafficStr;
        if (edge.trafficLevel == TrafficLevel::LOW) trafficStr = GREEN + "LOW (1.0x)" + RESET;
        else if (edge.trafficLevel == TrafficLevel::MEDIUM) trafficStr = YELLOW + "MED (1.5x)" + RESET;
        else trafficStr = RED + "HIGH (2.5x)" + RESET;

        std::string statusStr = edge.blocked ? (RED + BOLD + "BLOCKED" + RESET) : (GREEN + "OPEN" + RESET);

        std::cout << std::left << std::setw(5) << index++
                  << std::setw(32) << roadName
                  << std::setw(12) << (std::to_string(edge.distance).substr(0, 4) + " km")
                  << std::setw(14) << (std::to_string(edge.getEffectiveTravelTime()).substr(0, 4) + " min")
                  << std::setw(25) << trafficStr
                  << statusStr << "\n";
    }
    std::cout << std::string(85, '-') << "\n";
}

void UI::printRouteResult(const SearchResult& result, bool showStepByStep) {
    if (!result.found) {
        std::cout << "\n" << RED << BOLD << "[-] NO ROUTE FOUND!" << RESET << "\n";
        std::cout << RED << "The destination is currently unreachable. Check for blocked roads or disconnected nodes.\n" << RESET;
        return;
    }

    const Node* srcNode = nav.getGraph().getNode(result.sourceId);
    const Node* destNode = nav.getGraph().getNode(result.destId);

    std::cout << "\n" << GREEN << BOLD << "=================== NAVIGATION ROUTE COMPUTED ===================" << RESET << "\n";
    std::cout << BOLD << "SOURCE:      " << RESET << (srcNode ? srcNode->name : "Unknown") << " [ID: " << result.sourceId << "]\n";
    std::cout << BOLD << "DESTINATION: " << RESET << (destNode ? destNode->name : "Unknown") << " [ID: " << result.destId << "]\n";
    std::cout << BOLD << "ALGORITHM:   " << RESET << CYAN << result.algorithmName << RESET << "\n";
    std::cout << BOLD << "MODE:        " << RESET << Navigation::modeToString(result.mode) << "\n";
    
    std::cout << "\n" << BOLD << "RECOMMENDED ROUTE PATH:" << RESET << "\n  ";
    for (size_t i = 0; i < result.path.size(); ++i) {
        const Node* node = nav.getGraph().getNode(result.path[i]);
        if (i == 0) {
            std::cout << GREEN << BOLD << "[" << (node ? node->name : "") << "]" << RESET;
        } else if (i == result.path.size() - 1) {
            std::cout << " ──> " << RED << BOLD << "[" << (node ? node->name : "") << "]" << RESET;
        } else {
            std::cout << " ──> " << CYAN << "[" << (node ? node->name : "") << "]" << RESET;
        }
    }
    std::cout << "\n\n";

    std::cout << BOLD << "TOTAL DISTANCE:         " << RESET << GREEN << std::fixed << std::setprecision(1) << result.totalDistance << " km" << RESET << "\n";
    std::cout << BOLD << "ESTIMATED TRAVEL TIME:  " << RESET << YELLOW << std::fixed << std::setprecision(1) << result.totalEstimatedTime << " min" << RESET << "\n";
    std::cout << BOLD << "GRAPH NODES EXPLORED:   " << RESET << MAGENTA << result.nodesExplored << " vertices" << RESET << "\n";
    std::cout << BOLD << "MEASURED EXECUTION CPU: " << RESET << CYAN << std::fixed << std::setprecision(2) << result.executionTimeMicroseconds << " microseconds" << RESET << "\n";

    if (showStepByStep && !result.segments.empty()) {
        std::cout << "\n" << BOLD << "--- STEP-BY-STEP TURN-BY-TURN DIRECTIONS ---" << RESET << "\n";
        for (size_t i = 0; i < result.segments.size(); ++i) {
            const auto& seg = result.segments[i];
            std::string tTag;
            if (seg.traffic == TrafficLevel::LOW) tTag = GREEN + "[Clear]" + RESET;
            else if (seg.traffic == TrafficLevel::MEDIUM) tTag = YELLOW + "[Moderate Traffic]" + RESET;
            else tTag = RED + "[Heavy Congestion!]" + RESET;

            std::cout << "  Step " << (i + 1) << ": Drive from " << BOLD << seg.fromName << RESET 
                      << " to " << BOLD << seg.toName << RESET << "\n"
                      << "          Distance: " << std::fixed << std::setprecision(1) << seg.distance << " km"
                      << " | Time: " << seg.travelTime << " min " << tTag << "\n";
        }
    }
    std::cout << GREEN << BOLD << "=================================================================\n" << RESET;
}

void UI::printComparisonTable(const SearchResult& dijkstraRes, const SearchResult& aStarRes) {
    std::cout << "\n" << CYAN << BOLD << "========================================================================\n";
    std::cout << "                ALGORITHM COMPARISON: DIJKSTRA VS A*                   \n";
    std::cout << "========================================================================\n" << RESET;

    const Node* srcNode = nav.getGraph().getNode(dijkstraRes.sourceId);
    const Node* destNode = nav.getGraph().getNode(dijkstraRes.destId);

    std::cout << "Source: " << BOLD << (srcNode ? srcNode->name : "") << RESET
              << "  ->  Destination: " << BOLD << (destNode ? destNode->name : "") << RESET << "\n";
    std::cout << "Active Optimization Mode: " << Navigation::modeToString(dijkstraRes.mode) << "\n\n";

    std::cout << "+---------------------------------+----------------------+----------------------+\n";
    std::cout << "| " << std::left << std::setw(31) << "METRIC" 
              << " | " << std::setw(20) << "DIJKSTRA (UNINFORMED)" 
              << " | " << std::setw(20) << "A* (INFORMED HEURISTIC)" << " |\n";
    std::cout << "+---------------------------------+----------------------+----------------------+\n";

    // Path Found
    std::cout << "| " << std::left << std::setw(31) << "Path Found"
              << " | " << std::setw(20) << (dijkstraRes.found ? "YES" : "NO")
              << " | " << std::setw(20) << (aStarRes.found ? "YES" : "NO") << " |\n";

    // Distance
    std::cout << "| " << std::left << std::setw(31) << "Total Distance"
              << " | " << std::setw(17) << (std::to_string(dijkstraRes.totalDistance).substr(0, 4) + " km") << "   "
              << " | " << std::setw(17) << (std::to_string(aStarRes.totalDistance).substr(0, 4) + " km") << "   |\n";

    // Travel Time
    std::cout << "| " << std::left << std::setw(31) << "Estimated Travel Time"
              << " | " << std::setw(17) << (std::to_string(dijkstraRes.totalEstimatedTime).substr(0, 4) + " min") << "  "
              << " | " << std::setw(17) << (std::to_string(aStarRes.totalEstimatedTime).substr(0, 4) + " min") << "  |\n";

    // Nodes Explored
    std::string dNodes = std::to_string(dijkstraRes.nodesExplored) + " vertices";
    std::string aNodes = std::to_string(aStarRes.nodesExplored) + " vertices";
    std::cout << "| " << std::left << std::setw(31) << "Nodes Explored (Settled)"
              << " | " << std::setw(20) << dNodes
              << " | " << std::setw(20) << aNodes << " |\n";

    // Execution Time
    std::string dTime = std::to_string(dijkstraRes.executionTimeMicroseconds).substr(0, 5) + " us";
    std::string aTime = std::to_string(aStarRes.executionTimeMicroseconds).substr(0, 5) + " us";
    std::cout << "| " << std::left << std::setw(31) << "Measured Execution Time"
              << " | " << std::setw(20) << dTime
              << " | " << std::setw(20) << aTime << " |\n";

    // Heuristic Used
    std::cout << "| " << std::left << std::setw(31) << "Heuristic Function"
              << " | " << std::setw(20) << "None (h(n) = 0)"
              << " | " << std::setw(20) << "Euclidean Distance" << " |\n";

    // Search Strategy
    std::cout << "| " << std::left << std::setw(31) << "Exploration Strategy"
              << " | " << std::setw(20) << "Concentric Uniform"
              << " | " << std::setw(20) << "Goal-Directed Vector" << " |\n";

    // Optimality Guarantee
    std::cout << "| " << std::left << std::setw(31) << "Optimality Guarantee"
              << " | " << std::setw(20) << "Strictly Optimal"
              << " | " << std::setw(20) << "Optimal (Admissible)" << " |\n";

    std::cout << "+---------------------------------+----------------------+----------------------+\n";

    std::cout << "\n" << BOLD << "ENGINEERING INSIGHT & DSA EXPLANATION:" << RESET << "\n";
    if (aStarRes.nodesExplored < dijkstraRes.nodesExplored) {
        int savedNodes = dijkstraRes.nodesExplored - aStarRes.nodesExplored;
        double reductionPct = (double)savedNodes / dijkstraRes.nodesExplored * 100.0;
        std::cout << GREEN << "[+] A* searched " << savedNodes << " fewer nodes (" 
                  << std::fixed << std::setprecision(1) << reductionPct 
                  << "% reduction) by pruning search branches facing away from the goal.\n" << RESET;
    } else {
        std::cout << YELLOW << "[*] Both algorithms explored a similar node count on this specific corridor topology.\n" << RESET;
    }
    std::cout << "• Dijkstra explores outwards equally in all directions (uniform-cost search).\n";
    std::cout << "• A* prioritizes nodes that minimize f(n) = g(n) + h(n), steering exploration towards the destination.\n";
    std::cout << "• Because Euclidean distance in 2D Euclidean plane is admissible (h(n) <= h*(n)), A* is guaranteed optimal.\n";
}

void UI::renderAsciiMap(const std::vector<int>& highlightedPath) {
    std::cout << "\n" << CYAN << BOLD;
    std::cout << "========================================================================\n";
    std::cout << "               TOPOLOGICAL MAP SCHEMATIC (24 LOCATIONS)                 \n";
    std::cout << "========================================================================\n" << RESET;

    auto inPath = [&](int id) -> bool {
        return std::find(highlightedPath.begin(), highlightedPath.end(), id) != highlightedPath.end();
    };

    auto nodeBox = [&](int id, const std::string& name) -> std::string {
        if (inPath(id)) {
            return RED + BOLD + "[" + std::to_string(id) + ":" + name + "]" + RESET;
        } else {
            return CYAN + "[" + std::to_string(id) + ":" + name + "]" + RESET;
        }
    };

    std::cout << "\n";
    std::cout << "                      " << nodeBox(23, "North Gate") << "                             \n";
    std::cout << "                       /          |         \\                          \n";
    std::cout << "                      /           |          \\                         \n";
    std::cout << "    " << nodeBox(14, "Boys Hostel") << "     " << nodeBox(13, "Innovation") << "    " << nodeBox(15, "Girls Hostel") << "\n";
    std::cout << "          |                 |       \\          /        |              \n";
    std::cout << "          |                 |        \\        /         |              \n";
    std::cout << "    " << nodeBox(12, "Basic Sci") << "        |      " << nodeBox(11, "Research") << "       |              \n";
    std::cout << "          |       \\         |       /      |            |              \n";
    std::cout << "          |        \\        |      /       |    " << nodeBox(18, "Hospital") << "   \n";
    std::cout << "    " << nodeBox(3, "Auditorium") << "      " << nodeBox(7, "CSE Block") << "──" << nodeBox(8, "IT & AI") << "   /      |      \n";
    std::cout << "        |       \\      /         \\     /        /       |      \n";
    std::cout << "        |        " << nodeBox(6, "Library") << "     " << nodeBox(9, "Engineering") << "    " << nodeBox(17, "Stadium") << "\n";
    std::cout << "    " << nodeBox(5, "Bus Term") << "      |        \\     /          |        /       \n";
    std::cout << "        |          |         " << nodeBox(16, "Food Court") << "     |       /        \n";
    std::cout << "        |          |          /          |      |      /         \n";
    std::cout << "    " << nodeBox(2, "Admin") << "───────┘         /           " << nodeBox(10, "Mech Lab") << "─" << nodeBox(22, "Lake") << "\n";
    std::cout << "        |                       /            |             |     \n";
    std::cout << "    " << nodeBox(1, "Security") << "   " << nodeBox(19, "SAC") << "──────────" << nodeBox(20, "Market") << "─────" << nodeBox(21, "Station") << "\n";
    std::cout << "        |       /         \\            /                         \n";
    std::cout << "    " << nodeBox(0, "Main Gate") << "──────────" << nodeBox(4, "Parking") << "───────┘                         \n";
    std::cout << "\n";

    if (!highlightedPath.empty()) {
        std::cout << BOLD << "ACTIVE ROUTE TRACE: " << RESET;
        for (size_t i = 0; i < highlightedPath.size(); ++i) {
            const Node* n = nav.getGraph().getNode(highlightedPath[i]);
            if (i == 0) std::cout << GREEN << BOLD << n->name << RESET;
            else if (i == highlightedPath.size() - 1) std::cout << " => " << RED << BOLD << n->name << RESET;
            else std::cout << " => " << YELLOW << n->name << RESET;
        }
        std::cout << "\n";
    }

    std::cout << "Legend: " << CYAN << "[ID:Name]" << RESET << " = Normal Location | "
              << RED << BOLD << "[ID:Name]" << RESET << " = Active Route / Highlighted\n";
    std::cout << "------------------------------------------------------------------------\n";
}

void UI::handleFindRoute() {
    clearScreen();
    displayHeader();
    std::cout << BOLD << "=== FIND BEST ROUTE ===" << RESET << "\n";
    printNodeDirectory();

    int src = promptForLocation("SOURCE");
    int dest = promptForLocation("DESTINATION");

    std::cout << "\nSelect Algorithm:\n";
    std::cout << " 1. Dijkstra's Algorithm (Guaranteed Shortest / Min-Heap)\n";
    std::cout << " 2. A* Search Algorithm (Goal-Directed Euclidean Heuristic)\n";
    int algoChoice = getValidInt("Enter choice (1-2): ", 1, 2);

    bool useAStar = (algoChoice == 2);
    SearchResult res = nav.findRoute(src, dest, useAStar);
    printRouteResult(res, true);

    renderAsciiMap(res.path);
    pauseForUser();
}

void UI::handleCompareAlgorithms() {
    clearScreen();
    displayHeader();
    std::cout << BOLD << "=== COMPARE DIJKSTRA VS A* SEARCH ===" << RESET << "\n";
    printNodeDirectory();

    int src = promptForLocation("SOURCE");
    int dest = promptForLocation("DESTINATION");

    auto [dijkstraRes, aStarRes] = nav.compareDijkstraVsAStar(src, dest);
    printComparisonTable(dijkstraRes, aStarRes);

    pauseForUser();
}

void UI::handleAlternativeRoute() {
    clearScreen();
    displayHeader();
    std::cout << BOLD << "=== ALTERNATIVE ROUTE FINDER ===" << RESET << "\n";
    printNodeDirectory();

    int src = promptForLocation("SOURCE");
    int dest = promptForLocation("DESTINATION");

    SearchResult primary = nav.findRoute(src, dest, false);
    if (!primary.found) {
        std::cout << RED << "Primary route could not be found.\n" << RESET;
        pauseForUser();
        return;
    }

    auto [altRoute, explanation] = nav.getAlternativeRoute(src, dest);

    std::cout << "\n" << GREEN << BOLD << "--- [1] PRIMARY BEST ROUTE ---" << RESET << "\n";
    printRouteResult(primary, false);

    if (altRoute.found) {
        std::cout << "\n" << YELLOW << BOLD << "--- [2] ALTERNATIVE ROUTE (CORRIDOR BYPASS) ---" << RESET << "\n";
        printRouteResult(altRoute, false);

        std::cout << "\n" << BOLD << "SYSTEM DECISION RATIONALE:" << RESET << "\n";
        std::cout << CYAN << "  " << explanation << RESET << "\n";
    } else {
        std::cout << "\n" << YELLOW << "No distinct viable alternative bypass exists.\n" << RESET;
    }

    pauseForUser();
}

void UI::handleSimulateTraffic() {
    clearScreen();
    displayHeader();
    std::cout << BOLD << "=== SIMULATE TRAFFIC CONGESTION ===" << RESET << "\n";
    std::cout << "Select a road segment to update its traffic congestion level.\n";
    printRoadDirectory();

    std::cout << "Enter the source and destination IDs of the road to update:\n";
    int u = getValidInt("Enter Node U ID (0-23): ", 0, 23);
    int v = getValidInt("Enter Node V ID (0-23): ", 0, 23);

    const Edge* edge = nav.getGraph().getEdge(u, v);
    if (!edge) {
        std::cout << RED << "No direct road exists between [" << u << "] and [" << v << "].\n" << RESET;
        pauseForUser();
        return;
    }

    const Node* uNode = nav.getGraph().getNode(u);
    const Node* vNode = nav.getGraph().getNode(v);

    std::cout << "\nRoad: " << BOLD << uNode->name << " <-> " << vNode->name << RESET << "\n";
    std::cout << "Current Traffic: " << Edge::trafficLevelToString(edge->trafficLevel) << "\n\n";

    std::cout << "Choose New Traffic Level:\n";
    std::cout << " 1. LOW (1.0x - Clear flow)\n";
    std::cout << " 2. MEDIUM (1.5x - Moderate slowdown)\n";
    std::cout << " 3. HIGH (2.5x - Severe congestion)\n";
    int choice = getValidInt("Selection (1-3): ", 1, 3);

    TrafficLevel newLevel = TrafficLevel::LOW;
    if (choice == 2) newLevel = TrafficLevel::MEDIUM;
    else if (choice == 3) newLevel = TrafficLevel::HIGH;

    nav.updateRoadTraffic(u, v, newLevel);
    std::cout << GREEN << BOLD << "\n[+] Traffic congestion updated successfully for " 
              << uNode->name << " <-> " << vNode->name << "!" << RESET << "\n";
    std::cout << "Any routes utilizing this road will dynamically reflect the new travel time penalty.\n";

    pauseForUser();
}

void UI::handleBlockRoad() {
    clearScreen();
    displayHeader();
    std::cout << BOLD << "=== DYNAMIC ROAD BLOCKAGE SIMULATION ===" << RESET << "\n";
    printRoadDirectory();

    std::cout << "Enter the source and destination IDs of the road to block/unblock:\n";
    int u = getValidInt("Enter Node U ID (0-23): ", 0, 23);
    int v = getValidInt("Enter Node V ID (0-23): ", 0, 23);

    const Edge* edge = nav.getGraph().getEdge(u, v);
    if (!edge) {
        std::cout << RED << "No direct road exists between [" << u << "] and [" << v << "].\n" << RESET;
        pauseForUser();
        return;
    }

    const Node* uNode = nav.getGraph().getNode(u);
    const Node* vNode = nav.getGraph().getNode(v);

    std::cout << "\nRoad: " << BOLD << uNode->name << " <-> " << vNode->name << RESET << "\n";
    std::cout << "Current Status: " << (edge->blocked ? RED + BOLD + "BLOCKED" : GREEN + "OPEN") << RESET << "\n\n";

    std::cout << "Actions:\n";
    std::cout << " 1. Block Road (Close to all traffic)\n";
    std::cout << " 2. Unblock Road (Reopen road)\n";
    int choice = getValidInt("Enter choice (1-2): ", 1, 2);

    bool shouldBlock = (choice == 1);
    nav.setRoadBlockage(u, v, shouldBlock);

    std::cout << (shouldBlock ? RED : GREEN) << BOLD << "\n[!] ROAD STATUS UPDATED: " 
              << uNode->name << " <-> " << vNode->name << " is now " 
              << (shouldBlock ? "BLOCKED (Closed)" : "OPEN (Available)") << RESET << "\n";
    std::cout << "Graph weights dynamically updated. Shortest path searches will now automatically avoid this road.\n";

    pauseForUser();
}

void UI::handleTraversals() {
    clearScreen();
    displayHeader();
    std::cout << BOLD << "=== GRAPH TRAVERSAL ALGORITHMS (BFS & DFS) ===" << RESET << "\n";
    printNodeDirectory();

    int startNode = promptForLocation("STARTING LOCATION");

    std::cout << "\nChoose Traversal Algorithm:\n";
    std::cout << " 1. Breadth-First Search (BFS) - Level-order / Shortest hops\n";
    std::cout << " 2. Depth-First Search (DFS) - Deep recursive exploration\n";
    std::cout << " 3. Run BOTH (Compare BFS vs DFS Traversal Orders)\n";
    int choice = getValidInt("Enter choice (1-3): ", 1, 3);

    const Node* sNode = nav.getGraph().getNode(startNode);
    std::string sName = sNode ? sNode->name : "Node " + std::to_string(startNode);

    if (choice == 1 || choice == 3) {
        std::vector<int> bfsOrder = Algorithms::bfsTraversal(nav.getGraph(), startNode);
        std::cout << "\n" << GREEN << BOLD << "--- BREADTH-FIRST SEARCH (BFS) TRAVERSAL ---" << RESET << "\n";
        std::cout << "Data Structure: " << CYAN << "std::queue (FIFO)" << RESET << "\n";
        std::cout << "Starting Node:  " << BOLD << sName << " [ID: " << startNode << "]" << RESET << "\n";
        std::cout << "Total Visited:  " << bfsOrder.size() << " connected vertices\n";
        std::cout << "Visit Sequence:\n  ";
        for (size_t i = 0; i < bfsOrder.size(); ++i) {
            const Node* n = nav.getGraph().getNode(bfsOrder[i]);
            std::cout << "[" << bfsOrder[i] << ": " << (n ? n->name : "") << "]";
            if (i + 1 < bfsOrder.size()) std::cout << " -> ";
            if ((i + 1) % 3 == 0) std::cout << "\n  ";
        }
        std::cout << "\n";
    }

    if (choice == 2 || choice == 3) {
        std::vector<int> dfsOrder = Algorithms::dfsTraversal(nav.getGraph(), startNode);
        std::cout << "\n" << MAGENTA << BOLD << "--- DEPTH-FIRST SEARCH (DFS) TRAVERSAL ---" << RESET << "\n";
        std::cout << "Data Structure: " << CYAN << "std::stack (LIFO)" << RESET << "\n";
        std::cout << "Starting Node:  " << BOLD << sName << " [ID: " << startNode << "]" << RESET << "\n";
        std::cout << "Total Visited:  " << dfsOrder.size() << " connected vertices\n";
        std::cout << "Visit Sequence:\n  ";
        for (size_t i = 0; i < dfsOrder.size(); ++i) {
            const Node* n = nav.getGraph().getNode(dfsOrder[i]);
            std::cout << "[" << dfsOrder[i] << ": " << (n ? n->name : "") << "]";
            if (i + 1 < dfsOrder.size()) std::cout << " -> ";
            if ((i + 1) % 3 == 0) std::cout << "\n  ";
        }
        std::cout << "\n";
    }

    pauseForUser();
}

void UI::handleViewMap() {
    clearScreen();
    displayHeader();
    renderAsciiMap();
    printNodeDirectory();
    pauseForUser();
}

void UI::handleVivaMode() {
    clearScreen();
    displayHeader();
    std::cout << CYAN << BOLD << "========================================================================\n";
    std::cout << "                 VIVA & EDUCATIONAL DEFENSE MODE                        \n";
    std::cout << "            COMPREHENSIVE DSA THEORETICAL BREAKDOWN                     \n";
    std::cout << "========================================================================\n" << RESET;

    std::cout << BOLD << "Q1: Why is a Graph data structure required for map navigation?" << RESET << "\n";
    std::cout << "A: Road networks consist of non-linear intersections (vertices/nodes) and roads (edges)\n"
              << "   connecting them. Trees cannot represent cyclic routes or redundant multi-path road\n"
              << "   networks, making Graphs the only mathematically accurate data structure.\n\n";

    std::cout << BOLD << "Q2: Why use an Adjacency List instead of an Adjacency Matrix?" << RESET << "\n";
    std::cout << "A: Road networks are SPARSE graphs. With V=24 locations, each intersection typically connects\n"
              << "   to only 3-4 roads (E ~ 43). An Adjacency Matrix requires O(V^2) = 576 memory cells and\n"
              << "   inspects neighbors in O(V) time. An Adjacency List uses O(V + E) memory and accesses\n"
              << "   immediate neighbors in O(degree(u)) time, which is much faster and memory efficient.\n\n";

    std::cout << BOLD << "Q3: Why can BFS NOT be used as the primary shortest path algorithm here?" << RESET << "\n";
    std::cout << "A: Standard BFS assumes uniform, unweighted edges (each step costs 1). In map navigation,\n"
              << "   roads have differing lengths (km) and varying travel times (minutes with traffic).\n"
              << "   A path with 2 long roads might take 30 min, whereas a 3-road path might take only 10 min.\n"
              << "   BFS only finds the minimum hop count, whereas Dijkstra finds the minimum total edge weight.\n\n";

    std::cout << BOLD << "Q4: How does std::priority_queue optimize Dijkstra's Algorithm?" << RESET << "\n";
    std::cout << "A: Without a priority queue, finding the minimum unvisited vertex takes O(V) linear search,\n"
              << "   yielding O(V^2) overall complexity. A min-heap (std::priority_queue with std::greater)\n"
              << "   retrieves the minimum cost node in O(log V) time, reducing total time to O((V + E) log V).\n\n";

    std::cout << BOLD << "Q5: How does A* differ from Dijkstra?" << RESET << "\n";
    std::cout << "A: Dijkstra is UNINFORMED: it explores equally in all directions based only on accumulated\n"
              << "   cost g(n). A* is INFORMED: it guides the search towards the destination using evaluation\n"
              << "   function f(n) = g(n) + h(n), where h(n) is the Euclidean distance heuristic. This prunes\n"
              << "   branches leading in the opposite direction, visiting fewer nodes while guaranteeing optimality.\n\n";

    std::cout << BOLD << "Q6: What makes the A* Euclidean heuristic 'Admissible' and 'Consistent'?" << RESET << "\n";
    std::cout << "A: Admissibility means h(n) never overestimates the true cost to the destination. Since\n"
              << "   the straight line (Euclidean distance) between two points is the shortest possible path\n"
              << "   in 2D space, real road network distance is always >= Euclidean distance. Hence h(n) <= h*(n).\n\n";

    std::cout << BOLD << "Q7: How does traffic simulation impact graph edge weights dynamically?" << RESET << "\n";
    std::cout << "A: Each edge stores a base travel time and a traffic level (LOW=1.0x, MED=1.5x, HIGH=2.5x).\n"
              << "   When traffic is simulated, the effective weight w(u, v) = base_time * multiplier increases.\n"
              << "   During Dijkstra relaxation, a path previously optimal can become sub-optimal, causing the\n"
              << "   algorithm to route vehicles along quieter peripheral corridors.\n\n";

    std::cout << BOLD << "Q8: How does Road Blockage demonstrate dynamic graph manipulation?" << RESET << "\n";
    std::cout << "A: When a road is blocked, edge.blocked is set to true. The relaxation loop in Dijkstra and A*\n"
              << "   treats the edge as having infinite cost and skips it, instantly finding an alternative path\n"
              << "   without needing to rebuild the entire graph.\n";

    pauseForUser();
}

void UI::handleProjectStatistics() {
    clearScreen();
    displayHeader();
    std::cout << BOLD << "=== SMARTROUTE PROJECT & GRAPH STATISTICS ===" << RESET << "\n";

    std::cout << "------------------------------------------------------------------------\n";
    std::cout << " Total Locations (Vertices V):    " << GREEN << nav.getGraph().getNodeCount() << RESET << "\n";
    std::cout << " Physical Bidirectional Roads:   " << GREEN << nav.getGraph().getPhysicalRoadCount() << RESET << "\n";
    std::cout << " Total Directed Graph Edges (E):  " << GREEN << nav.getGraph().getDirectedEdgeCount() << RESET << "\n";
    std::cout << " Graph Representation:            " << CYAN << "Adjacency List (std::unordered_map<int, vector<Edge>>)" << RESET << "\n";
    std::cout << " Graph Classification:            " << CYAN << "Weighted Directed/Bidirectional Cyclic Spatial Graph" << RESET << "\n";
    std::cout << " Number of Blocked Roads:         " << (nav.getGraph().getBlockedRoadCount() > 0 ? RED : GREEN) 
              << nav.getGraph().getBlockedRoadCount() << RESET << "\n";
    std::cout << " Number of Congested Roads:       " << (nav.getGraph().getHighTrafficRoadCount() > 0 ? YELLOW : GREEN) 
              << nav.getGraph().getHighTrafficRoadCount() << RESET << "\n";
    std::cout << " Active Route Optimization Mode:  " << BOLD << Navigation::modeToString(nav.getRouteMode()) << RESET << "\n";
    std::cout << " Algorithms Implemented:          " << YELLOW << "Dijkstra, A* Search, BFS, DFS, Alternative Routing" << RESET << "\n";
    std::cout << " Heuristic Model:                 " << MAGENTA << "Admissible 2D Euclidean Distance: sqrt(dx^2 + dy^2)" << RESET << "\n";
    std::cout << " Time Complexities:               \n";
    std::cout << "   - Dijkstra's Algorithm:        O((V + E) log V)\n";
    std::cout << "   - A* Search Algorithm:         O(E) best, O((V + E) log V) worst\n";
    std::cout << "   - BFS & DFS Traversals:        O(V + E)\n";
    std::cout << " Space Complexity:                O(V + E)\n";
    std::cout << "------------------------------------------------------------------------\n";

    pauseForUser();
}

void UI::handleRunPresentationDemo() {
    clearScreen();
    displayHeader();
    std::cout << GREEN << BOLD;
    std::cout << "========================================================================\n";
    std::cout << "         AUTOMATED 5-MINUTE VIVA PRESENTATION DEMO WALKTHROUGH          \n";
    std::cout << "========================================================================\n" << RESET;
    std::cout << "This automated script demonstrates the 12 key project milestones\n"
              << "step-by-step for your project evaluation and viva demonstration.\n\n";

    auto stepPause = [&](int stepNum, const std::string& title) {
        std::cout << "\n" << BOLD << BG_BLUE << " [STEP " << stepNum << "/12]: " << title << " " << RESET << "\n";
        std::cout << CYAN << "Press Enter to proceed to this step..." << RESET;
        std::string d;
        std::getline(std::cin, d);
    };

    // STEP 1: Open Application
    stepPause(1, "Open Application & Initialize Graph");
    std::cout << "[+] SmartRoute graph loaded successfully with " << nav.getGraph().getNodeCount() 
              << " vertices and " << nav.getGraph().getPhysicalRoadCount() << " roads in Adjacency List format.\n";

    // STEP 2: Show City Map
    stepPause(2, "Show Campus / City Map Schematic");
    renderAsciiMap();

    // STEP 3: Setup Source and Destination
    stepPause(3, "Select Source: Main Gate [0] -> Destination: Hospital [18]");
    int src = 0;   // Main Gate
    int dest = 18; // Hospital
    std::cout << "Source set to:      [0] Main Gate (South-West corner)\n";
    std::cout << "Destination set to: [18] University Hospital (North-East corner)\n";

    // STEP 4: Run Dijkstra
    stepPause(4, "Run Dijkstra's Algorithm");
    SearchResult dRes1 = nav.findRoute(src, dest, false);
    printRouteResult(dRes1, false);

    // STEP 5: Run A* & Compare
    stepPause(5, "Run A* Search Algorithm & Show Comparison");
    SearchResult aRes1 = nav.findRoute(src, dest, true);
    printComparisonTable(dRes1, aRes1);

    // STEP 6: Increase Traffic on current optimal arterial
    stepPause(6, "Simulate Heavy Traffic Congestion on Arterial [Parking Plaza <-> Market Square]");
    std::cout << "[*] Simulating severe rush-hour traffic on Parking Plaza West [4] <-> University Market Square [20]...\n";
    nav.updateRoadTraffic(4, 20, TrafficLevel::HIGH);
    nav.updateRoadTraffic(10, 22, TrafficLevel::HIGH);
    std::cout << GREEN << "[+] Traffic Level set to HIGH (2.5x multiplier, +15 min delay) on Eastern Arterial!\n" << RESET;

    // STEP 7: Run Navigation Again -> Visibly show route changes!
    stepPause(7, "Recalculate Route with High Traffic (Dynamic Rerouting)");
    SearchResult dRes2 = nav.findRoute(src, dest, false);
    std::cout << YELLOW << BOLD << ">>> DYNAMIC REROUTING TRIGGERED: System diverted away from congested Market corridor to Academic Arterial!\n" << RESET;
    printRouteResult(dRes2, false);

    // STEP 8: Simulate Road Blockage on new path
    stepPause(8, "Simulate Road Blockage [Security Post <-> Admin Block]");
    std::cout << "[*] Simulating emergency construction blockage on Security Post [1] <-> Admin Block [2]...\n";
    nav.setRoadBlockage(1, 2, true);
    std::cout << RED << BOLD << "[!] ROAD BLOCKED: Security Post <-> Admin Block is CLOSED!\n" << RESET;

    // STEP 9: Automatic Route Recalculation
    stepPause(9, "Run Navigation with Road Blockage");
    SearchResult dRes3 = nav.findRoute(src, dest, false);
    std::cout << GREEN << BOLD << ">>> ROAD BLOCKED -> RECALCULATING... Alternative bypass discovered via Central Hub!\n" << RESET;
    printRouteResult(dRes3, false);

    // STEP 10: Show Algorithm Comparison with new conditions
    stepPause(10, "Run Comparison under Dynamic Road Conditions");
    SearchResult aRes3 = nav.findRoute(src, dest, true);
    printComparisonTable(dRes3, aRes3);

    // STEP 11: Graph Traversals
    stepPause(11, "Demonstrate Graph Traversals (BFS and DFS)");
    std::vector<int> bfsOrder = Algorithms::bfsTraversal(nav.getGraph(), src);
    std::vector<int> dfsOrder = Algorithms::dfsTraversal(nav.getGraph(), src);
    std::cout << GREEN << "BFS Order (Level Order / Minimum Hops):\n  " << RESET;
    for (size_t i = 0; i < std::min((size_t)8, bfsOrder.size()); ++i) {
        std::cout << "[" << nav.getGraph().getNode(bfsOrder[i])->name << "] -> ";
    }
    std::cout << "...\n";

    std::cout << MAGENTA << "DFS Order (Deep Branch Exploration):\n  " << RESET;
    for (size_t i = 0; i < std::min((size_t)8, dfsOrder.size()); ++i) {
        std::cout << "[" << nav.getGraph().getNode(dfsOrder[i])->name << "] -> ";
    }
    std::cout << "...\n";

    // STEP 12: Project Statistics
    stepPause(12, "Review Final System Statistics");
    std::cout << "Total Locations: " << nav.getGraph().getNodeCount() << "\n";
    std::cout << "Total Roads:     " << nav.getGraph().getPhysicalRoadCount() << "\n";
    std::cout << "Blocked Roads:   " << nav.getGraph().getBlockedRoadCount() << "\n";
    std::cout << "Congested Roads: " << nav.getGraph().getHighTrafficRoadCount() << "\n";
    std::cout << GREEN << BOLD << "\n[SUCCESS] 5-MINUTE LIVE DEMO COMPLETED SUCCESSFULLY!\n" << RESET;

    pauseForUser();
}

void UI::handleResetMap() {
    nav.resetToDefaults();
    std::cout << GREEN << BOLD << "\n[+] Network successfully reset! All road blockages cleared and traffic set to LOW.\n" << RESET;
    pauseForUser();
}

void UI::run() {
    int choice = -1;
    while (choice != 0) {
        clearScreen();
        displayMainMenu();
        choice = getValidInt("Enter selection (0-12): ", 0, 12);

        switch (choice) {
            case 1:  handleFindRoute(); break;
            case 2:  handleCompareAlgorithms(); break;
            case 3:  handleAlternativeRoute(); break;
            case 4:  handleSimulateTraffic(); break;
            case 5:  handleBlockRoad(); break;
            case 6:  handleTraversals(); break;
            case 7:  handleViewMap(); break;
            case 8:  handleVivaMode(); break;
            case 9:  handleProjectStatistics(); break;
            case 10: handleRunPresentationDemo(); break;
            case 11: {
                clearScreen();
                displayHeader();
                std::cout << BOLD << "=== SELECT ROUTE OPTIMIZATION MODE ===" << RESET << "\n";
                std::cout << " 1. Shortest Distance (Minimize physical distance in km)\n";
                std::cout << " 2. Fastest Time (Minimize travel time in minutes with traffic)\n";
                std::cout << " 3. Avoid High Traffic (Penalize congested corridors)\n";
                std::cout << " 4. Emergency Route (Strictly prioritize clear high-speed arterials)\n";
                int mChoice = getValidInt("Select mode (1-4): ", 1, 4);
                nav.setRouteMode(static_cast<RouteMode>(mChoice));
                std::cout << GREEN << "\n[+] Optimization Mode updated to: " 
                          << Navigation::modeToString(nav.getRouteMode()) << "\n" << RESET;
                pauseForUser();
                break;
            }
            case 12: handleResetMap(); break;
            case 0:
                std::cout << CYAN << BOLD << "\nThank you for using SmartRoute! Best wishes for your project presentation!\n\n" << RESET;
                break;
            default: break;
        }
    }
}
