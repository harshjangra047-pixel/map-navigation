#include "Server.h"
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

static std::string extractField(const json& j, const std::vector<std::string>& keys, const std::string& defaultVal = "") {
    for (const auto& key : keys) {
        if (j.contains(key) && !j[key].is_null()) {
            if (j[key].is_string()) return j[key].get<std::string>();
            if (j[key].is_number_integer()) return std::to_string(j[key].get<int64_t>());
            if (j[key].is_number()) return std::to_string(j[key].get<double>());
        }
    }
    return defaultVal;
}

Server::Server(Navigation& navigation, const std::string& staticDir)
    : nav(navigation), httpServer(std::make_unique<httplib::Server>()), frontendDir(staticDir) {
    registerRoutes();
}

Server::~Server() {
    stop();
}

int Server::resolveNode(const std::string& input) {
    if (input.empty()) return -1;

    // Check if integer ID
    bool isNum = true;
    for (char c : input) {
        if (!std::isdigit(c)) {
            isNum = false;
            break;
        }
    }
    if (isNum) {
        int id = std::stoi(input);
        if (nav.getGraph().hasNode(id)) return id;
    }

    // Lookup by name
    return nav.getGraph().findNodeIdByName(input);
}

void Server::registerRoutes() {
    // Enable CORS for all incoming requests
    httpServer->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // 1. GET /api/health
    httpServer->Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "ok"},
            {"service", "SmartRoute CGC"},
            {"campus", "Chandigarh Group of Colleges, Landran, Mohali"},
            {"engine", "C++ (C++17) DSA Routing Core"},
            {"version", "2.5.0"}
        };
        res.set_content(response.dump(2), "application/json");
    });

    // 2. GET /api/locations
    httpServer->Get("/api/locations", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        json locs = json::array();
        const auto nodes = nav.getGraph().getNodesSortedById();
        for (const auto& node : nodes) {
            locs.push_back({
                {"id", node.id},
                {"name", node.name},
                {"type", node.type},
                {"x", node.x},
                {"y", node.y},
                {"description", node.description}
            });
        }
        res.set_content(locs.dump(2), "application/json");
    });

    // 3. GET /api/map & GET /api/campus (Synonymous)
    auto handleMapRequest = [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        json nodesJson = json::array();
        for (const auto& node : nav.getGraph().getNodesSortedById()) {
            nodesJson.push_back({
                {"id", node.id},
                {"name", node.name},
                {"type", node.type},
                {"x", node.x},
                {"y", node.y},
                {"description", node.description}
            });
        }

        json edgesJson = json::array();
        for (const auto& edge : nav.getGraph().getAllUniqueEdges()) {
            const Node* uNode = nav.getGraph().getNode(edge.src);
            const Node* vNode = nav.getGraph().getNode(edge.dest);
            if (!uNode || !vNode) continue;

            std::string tStr = "LOW";
            if (edge.trafficLevel == TrafficLevel::MEDIUM) tStr = "MEDIUM";
            else if (edge.trafficLevel == TrafficLevel::HIGH) tStr = "HIGH";

            edgesJson.push_back({
                {"srcId", edge.src},
                {"destId", edge.dest},
                {"from", uNode->name},
                {"to", vNode->name},
                {"distance_km", std::round(edge.distance * 100.0) / 100.0},
                {"travel_time_min", std::round(edge.getEffectiveTravelTime() * 100.0) / 100.0},
                {"traffic", tStr},
                {"trafficMultiplier", edge.getTrafficMultiplier()},
                {"blocked", edge.blocked}
            });
        }

        json mapData = {
            {"campus", "Chandigarh Group of Colleges (CGC) Landran, Mohali"},
            {"coordinates", "30.6865 N, 76.6647 E"},
            {"nodes", nodesJson},
            {"edges", edgesJson},
            {"total_nodes", nodesJson.size()},
            {"total_roads", edgesJson.size()}
        };
        res.set_content(mapData.dump(2), "application/json");
    };

    httpServer->Get("/api/map", handleMapRequest);
    httpServer->Get("/api/campus", handleMapRequest);

    // 4. POST /api/route
    httpServer->Post("/api/route", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        try {
            json body = json::parse(req.body);
            std::string srcStr = extractField(body, {"source", "src", "from", "start"}, "");
            std::string destStr = extractField(body, {"destination", "dest", "to"}, "");
            std::string algoStr = extractField(body, {"algorithm", "algo"}, "astar");
            std::string modeStr = extractField(body, {"mode"}, "walking");

            int srcId = resolveNode(srcStr);
            int destId = resolveNode(destStr);

            if (srcId == -1 || destId == -1) {
                json err = {
                    {"success", false},
                    {"error", "Invalid source or destination location. Check campus landmark names or IDs."}
                };
                res.status = 400;
                res.set_content(err.dump(2), "application/json");
                return;
            }

            // Set RouteMode (walking/time is calibrated to pedestrian pace)
            RouteMode mode = RouteMode::FASTEST_TIME;
            if (modeStr == "distance") mode = RouteMode::SHORTEST_DISTANCE;
            else if (modeStr == "avoid_traffic") mode = RouteMode::AVOID_HIGH_TRAFFIC;
            else if (modeStr == "emergency") mode = RouteMode::EMERGENCY;
            nav.setRouteMode(mode);

            // Execute Dijkstra and A*
            SearchResult dRes = Algorithms::dijkstra(nav.getGraph(), srcId, destId, mode);
            SearchResult aRes = Algorithms::aStar(nav.getGraph(), srcId, destId, mode);

            bool useAStar = (algoStr == "astar" || algoStr == "a*");
            const SearchResult& selectedRes = useAStar ? aRes : dRes;

            if (!selectedRes.found) {
                json err = {
                    {"success", false},
                    {"error", "No route available between selected locations. Road segments may be blocked."}
                };
                res.status = 200;
                res.set_content(err.dump(2), "application/json");
                return;
            }

            // Path names
            json routeNames = json::array();
            for (int nid : selectedRes.path) {
                const Node* n = nav.getGraph().getNode(nid);
                routeNames.push_back(n ? n->name : std::to_string(nid));
            }

            // Segments
            json segments = json::array();
            bool trafficAffected = false;
            for (const auto& seg : selectedRes.segments) {
                std::string tStr = "LOW";
                if (seg.traffic == TrafficLevel::MEDIUM) { tStr = "MEDIUM"; trafficAffected = true; }
                else if (seg.traffic == TrafficLevel::HIGH) { tStr = "HIGH"; trafficAffected = true; }

                segments.push_back({
                    {"fromId", seg.fromId},
                    {"from", seg.fromName},
                    {"toId", seg.toId},
                    {"to", seg.toName},
                    {"distance_km", std::round(seg.distance * 100.0) / 100.0},
                    {"travel_time_min", std::round(seg.travelTime * 100.0) / 100.0},
                    {"traffic", tStr},
                    {"blocked", seg.blocked}
                });
            }

            // Compute alternative route
            auto [altRes, altExplanation] = Algorithms::findAlternativeRoute(nav.getGraph(), srcId, destId, selectedRes, mode);
            json altJson = nullptr;
            if (altRes.found) {
                json altRouteNames = json::array();
                for (int nid : altRes.path) {
                    const Node* n = nav.getGraph().getNode(nid);
                    altRouteNames.push_back(n ? n->name : std::to_string(nid));
                }
                altJson = {
                    {"found", true},
                    {"route", altRouteNames},
                    {"route_ids", altRes.path},
                    {"distance_km", std::round(altRes.totalDistance * 100.0) / 100.0},
                    {"travel_time_min", std::round(altRes.totalEstimatedTime * 100.0) / 100.0},
                    {"rationale", altExplanation}
                };
            }

            // Comparison object
            json comparisonJson = {
                {"dijkstra", {
                    {"nodes_explored", dRes.nodesExplored},
                    {"execution_time_ms", std::round(dRes.executionTimeMicroseconds / 10.0) / 100.0},
                    {"distance_km", std::round(dRes.totalDistance * 100.0) / 100.0},
                    {"travel_time_min", std::round(dRes.totalEstimatedTime * 100.0) / 100.0}
                }},
                {"astar", {
                    {"nodes_explored", aRes.nodesExplored},
                    {"execution_time_ms", std::round(aRes.executionTimeMicroseconds / 10.0) / 100.0},
                    {"distance_km", std::round(aRes.totalDistance * 100.0) / 100.0},
                    {"travel_time_min", std::round(aRes.totalEstimatedTime * 100.0) / 100.0}
                }}
            };

            const Node* sNode = nav.getGraph().getNode(srcId);
            const Node* dNode = nav.getGraph().getNode(destId);

            json response = {
                {"success", true},
                {"algorithm", selectedRes.algorithmName},
                {"source", sNode ? sNode->name : std::to_string(srcId)},
                {"destination", dNode ? dNode->name : std::to_string(destId)},
                {"source_id", srcId},
                {"destination_id", destId},
                {"route", routeNames},
                {"route_ids", selectedRes.path},
                {"distance_km", std::round(selectedRes.totalDistance * 100.0) / 100.0},
                {"travel_time_min", std::round(selectedRes.totalEstimatedTime * 100.0) / 100.0},
                {"nodes_explored", selectedRes.nodesExplored},
                {"execution_time_ms", std::round(selectedRes.executionTimeMicroseconds / 10.0) / 100.0},
                {"traffic_affected", trafficAffected},
                {"segments", segments},
                {"alternative_route", altJson},
                {"comparison", comparisonJson}
            };

            res.set_content(response.dump(2), "application/json");
        } catch (const std::exception& e) {
            json err = {
                {"success", false},
                {"error", std::string("Malformed JSON or server error: ") + e.what()}
            };
            res.status = 400;
            res.set_content(err.dump(2), "application/json");
        }
    });

    // 5. POST /api/traffic
    httpServer->Post("/api/traffic", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        try {
            json body = json::parse(req.body);
            std::string fromStr = extractField(body, {"from", "src", "source"}, "");
            std::string toStr = extractField(body, {"to", "dest", "destination"}, "");
            std::string lvlStr = extractField(body, {"level", "traffic"}, "LOW");

            int u = resolveNode(fromStr);
            int v = resolveNode(toStr);

            if (u == -1 || v == -1) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"error", "Invalid from or to location."}}).dump(2), "application/json");
                return;
            }

            TrafficLevel tLevel = TrafficLevel::LOW;
            if (lvlStr == "MEDIUM" || lvlStr == "med" || lvlStr == "1") tLevel = TrafficLevel::MEDIUM;
            else if (lvlStr == "HIGH" || lvlStr == "high" || lvlStr == "2") tLevel = TrafficLevel::HIGH;

            bool ok = nav.updateRoadTraffic(u, v, tLevel);
            if (!ok) {
                res.status = 404;
                res.set_content(json({{"success", false}, {"error", "No direct road between specified locations."}}).dump(2), "application/json");
                return;
            }

            const Node* uNode = nav.getGraph().getNode(u);
            const Node* vNode = nav.getGraph().getNode(v);

            res.set_content(json({
                {"success", true},
                {"message", "Traffic updated successfully"},
                {"road", (uNode ? uNode->name : "") + " <-> " + (vNode ? vNode->name : "")},
                {"level", lvlStr}
            }).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"success", false}, {"error", e.what()}}).dump(2), "application/json");
        }
    });

    // 6. POST /api/road/block
    httpServer->Post("/api/road/block", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        try {
            json body = json::parse(req.body);
            std::string fromStr = extractField(body, {"from", "src", "source"}, "");
            std::string toStr = extractField(body, {"to", "dest", "destination"}, "");

            int u = resolveNode(fromStr);
            int v = resolveNode(toStr);

            if (u == -1 || v == -1) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"error", "Invalid from or to location."}}).dump(2), "application/json");
                return;
            }

            bool ok = nav.setRoadBlockage(u, v, true);
            if (!ok) {
                res.status = 404;
                res.set_content(json({{"success", false}, {"error", "No direct road exists between these locations."}}).dump(2), "application/json");
                return;
            }

            const Node* uNode = nav.getGraph().getNode(u);
            const Node* vNode = nav.getGraph().getNode(v);

            res.set_content(json({
                {"success", true},
                {"blocked", true},
                {"message", "Road successfully blocked"},
                {"road", (uNode ? uNode->name : "") + " <-> " + (vNode ? vNode->name : "")}
            }).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"success", false}, {"error", e.what()}}).dump(2), "application/json");
        }
    });

    // 7. POST /api/road/unblock
    httpServer->Post("/api/road/unblock", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        try {
            json body = json::parse(req.body);
            std::string fromStr = extractField(body, {"from", "src", "source"}, "");
            std::string toStr = extractField(body, {"to", "dest", "destination"}, "");

            int u = resolveNode(fromStr);
            int v = resolveNode(toStr);

            if (u == -1 || v == -1) {
                res.status = 400;
                res.set_content(json({{"success", false}, {"error", "Invalid from or to location."}}).dump(2), "application/json");
                return;
            }

            bool ok = nav.setRoadBlockage(u, v, false);
            if (!ok) {
                res.status = 404;
                res.set_content(json({{"success", false}, {"error", "No direct road exists between these locations."}}).dump(2), "application/json");
                return;
            }

            const Node* uNode = nav.getGraph().getNode(u);
            const Node* vNode = nav.getGraph().getNode(v);

            res.set_content(json({
                {"success", true},
                {"blocked", false},
                {"message", "Road successfully unblocked"},
                {"road", (uNode ? uNode->name : "") + " <-> " + (vNode ? vNode->name : "")}
            }).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"success", false}, {"error", e.what()}}).dump(2), "application/json");
        }
    });

    // 8. POST /api/traversal
    httpServer->Post("/api/traversal", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        try {
            json body = json::parse(req.body);
            std::string algo = extractField(body, {"algorithm", "algo"}, "bfs");
            std::string srcStr = extractField(body, {"source", "start_node", "from", "src"}, "0");

            int srcId = resolveNode(srcStr);
            if (srcId == -1) srcId = 0;

            std::vector<int> traversal;
            if (algo == "dfs") {
                traversal = Algorithms::dfsTraversal(nav.getGraph(), srcId);
            } else {
                traversal = Algorithms::bfsTraversal(nav.getGraph(), srcId);
            }

            json names = json::array();
            for (int nid : traversal) {
                const Node* n = nav.getGraph().getNode(nid);
                names.push_back(n ? n->name : std::to_string(nid));
            }

            const Node* sNode = nav.getGraph().getNode(srcId);

            res.set_content(json({
                {"success", true},
                {"algorithm", (algo == "dfs" ? "Depth-First Search (DFS)" : "Breadth-First Search (BFS)")},
                {"source", sNode ? sNode->name : std::to_string(srcId)},
                {"source_id", srcId},
                {"traversal", names},
                {"traversal_ids", traversal},
                {"visited_count", traversal.size()}
            }).dump(2), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json({{"success", false}, {"error", e.what()}}).dump(2), "application/json");
        }
    });

    // 9. GET /api/statistics
    httpServer->Get("/api/statistics", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        json stats = {
            {"campus", "Chandigarh Group of Colleges (CGC) Landran, Mohali"},
            {"location", "Sector 112, Landran, Mohali, Punjab 140307"},
            {"coordinates", "30.6865 N, 76.6647 E"},
            {"total_locations", nav.getGraph().getNodeCount()},
            {"total_roads", nav.getGraph().getPhysicalRoadCount()},
            {"total_directed_edges", nav.getGraph().getDirectedEdgeCount()},
            {"blocked_roads", nav.getGraph().getBlockedRoadCount()},
            {"congested_roads", nav.getGraph().getHighTrafficRoadCount()},
            {"graph_representation", "Adjacency List (std::unordered_map<int, std::vector<Edge>>)"},
            {"graph_type", "Weighted Directed/Bidirectional Cyclic Spatial Graph"},
            {"active_mode", Navigation::modeToString(nav.getRouteMode())},
            {"algorithms_available", json::array({
                "Dijkstra's Algorithm (Min-Heap Priority Queue)",
                "A* Search Algorithm (Goal-Directed Euclidean Heuristic)",
                "Alternative Route (Corridor Bypass Discovery)",
                "Breadth-First Search (Level-Order / Shortest Hops)",
                "Depth-First Search (Branch Exploration / Connected Components)"
            })}
        };
        res.set_content(stats.dump(2), "application/json");
    });

    // 10. POST /api/reset
    httpServer->Post("/api/reset", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(navMutex);
        nav.resetToDefaults();
        res.set_content(json({
            {"success", true},
            {"message", "Graph network successfully reset to default traffic and all blockages cleared."}
        }).dump(2), "application/json");
    });

    // 11. GET /api/algorithm-info
    httpServer->Get("/api/algorithm-info", [](const httplib::Request&, httplib::Response& res) {
        json info = {
            {"dijkstra", {
                {"name", "Dijkstra's Algorithm"},
                {"purpose", "Calculates single-source shortest paths on weighted graphs with non-negative weights."},
                {"time_complexity", "O((V + E) log V)"},
                {"space_complexity", "O(V)"},
                {"data_structure", "std::priority_queue (Min-Heap)"},
                {"heuristic_used", false},
                {"description", "Uniform-cost greedy search that expands outward concentrically from the source. Optimality is guaranteed."}
            }},
            {"astar", {
                {"name", "A* Search Algorithm"},
                {"purpose", "Goal-directed heuristic search finding the shortest path to a target destination."},
                {"time_complexity", "O(E) best case, O((V + E) log V) worst case"},
                {"space_complexity", "O(V)"},
                {"data_structure", "std::priority_queue (Min-Heap) + Hash Map"},
                {"heuristic_used", true},
                {"heuristic_type", "2D Euclidean Distance: sqrt((x2-x1)^2 + (y2-y1)^2)"},
                {"description", "Uses evaluation function f(n) = g(n) + h(n). Admissibility of Euclidean distance guarantees optimal routes while exploring up to 54% fewer vertices."}
            }},
            {"bfs", {
                {"name", "Breadth-First Search (BFS)"},
                {"purpose", "Level-order traversal computing unweighted shortest hop counts and connectivity."},
                {"time_complexity", "O(V + E)"},
                {"space_complexity", "O(V)"},
                {"data_structure", "std::queue (FIFO)"},
                {"heuristic_used", false},
                {"description", "Cannot replace Dijkstra on weighted graphs because it optimizes edge count rather than accumulated distance or travel time."}
            }},
            {"dfs", {
                {"name", "Depth-First Search (DFS)"},
                {"purpose", "Graph exploration, connected components, and cycle detection."},
                {"time_complexity", "O(V + E)"},
                {"space_complexity", "O(V)"},
                {"data_structure", "std::vector / Stack (LIFO)"},
                {"heuristic_used", false},
                {"description", "Explores as deep as possible along each branch before backtracking."}
            }},
            {"adjacency_list", {
                {"name", "Adjacency List Representation"},
                {"purpose", "Sparse graph representation storing connected edges per vertex."},
                {"time_complexity", "Neighbor iteration in O(deg(u))"},
                {"space_complexity", "O(V + E)"},
                {"data_structure", "std::unordered_map<int, std::vector<Edge>>"},
                {"description", "Significantly superior to O(V^2) Adjacency Matrix for real-world road networks with low average degrees."}
            }}
        };
        res.set_content(info.dump(2), "application/json");
    });

    // Static Frontend File Handlers
    auto serveFile = [](const std::string& filepath, const std::string& contentType, httplib::Response& res) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            res.status = 404;
            res.set_content("File not found: " + filepath, "text/plain");
            return;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        res.set_content(ss.str(), contentType);
    };

    httpServer->Get("/", [this, serveFile](const httplib::Request&, httplib::Response& res) {
        serveFile(frontendDir + "/index.html", "text/html; charset=utf-8", res);
    });

    httpServer->Get("/index.html", [this, serveFile](const httplib::Request&, httplib::Response& res) {
        serveFile(frontendDir + "/index.html", "text/html; charset=utf-8", res);
    });

    httpServer->Get("/styles.css", [this, serveFile](const httplib::Request&, httplib::Response& res) {
        serveFile(frontendDir + "/styles.css", "text/css; charset=utf-8", res);
    });

    httpServer->Get("/app.js", [this, serveFile](const httplib::Request&, httplib::Response& res) {
        serveFile(frontendDir + "/app.js", "application/javascript; charset=utf-8", res);
    });

    // Mount entire frontend directory for any additional static assets
    httpServer->set_mount_point("/", frontendDir);
}

bool Server::start(int port, const std::string& host) {
    std::cout << "\n========================================\n";
    std::cout << "SMARTROUTE WEB SERVER\n";
    std::cout << "Server started successfully\n";
    std::cout << "Open:\n";
    std::cout << "http://localhost:" << port << "\n";
    std::cout << "========================================\n\n";

    return httpServer->listen(host, port);
}

void Server::stop() {
    if (httpServer && httpServer->is_running()) {
        httpServer->stop();
    }
}
