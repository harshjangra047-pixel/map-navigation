#include "Graph.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

Graph::Graph() : totalRoadCount(0) {}

void Graph::addNode(const Node& node) {
    nodes[node.id] = node;
    if (adj.find(node.id) == adj.end()) {
        adj[node.id] = std::vector<Edge>();
    }
}

void Graph::addEdge(int src, int dest, double distance, double baseTime, 
                    TrafficLevel traffic, bool bidirectional) {
    // Add forward edge
    adj[src].push_back(Edge(src, dest, distance, baseTime, traffic, false));

    if (bidirectional) {
        // Add reverse edge
        adj[dest].push_back(Edge(dest, src, distance, baseTime, traffic, false));
    }
    totalRoadCount++;
}

bool Graph::hasNode(int id) const {
    return nodes.find(id) != nodes.end();
}

const Node* Graph::getNode(int id) const {
    auto it = nodes.find(id);
    if (it != nodes.end()) {
        return &(it->second);
    }
    return nullptr;
}

const Node* Graph::getNodeByName(const std::string& name) const {
    for (const auto& pair : nodes) {
        if (pair.second.name == name) {
            return &(pair.second);
        }
    }
    return nullptr;
}

int Graph::findNodeIdByName(const std::string& name) const {
    // Case-insensitive substring match or exact match
    std::string lowerQuery = name;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    // First try exact match
    for (const auto& pair : nodes) {
        std::string lowerNodeName = pair.second.name;
        std::transform(lowerNodeName.begin(), lowerNodeName.end(), lowerNodeName.begin(), ::tolower);
        if (lowerNodeName == lowerQuery) {
            return pair.first;
        }
    }

    // Next try substring search
    for (const auto& pair : nodes) {
        std::string lowerNodeName = pair.second.name;
        std::transform(lowerNodeName.begin(), lowerNodeName.end(), lowerNodeName.begin(), ::tolower);
        if (lowerNodeName.find(lowerQuery) != std::string::npos) {
            return pair.first;
        }
    }

    return -1;
}

const std::vector<Edge>& Graph::getNeighbors(int id) const {
    static const std::vector<Edge> emptyList;
    auto it = adj.find(id);
    if (it != adj.end()) {
        return it->second;
    }
    return emptyList;
}

const std::unordered_map<int, Node>& Graph::getAllNodes() const {
    return nodes;
}

std::vector<Node> Graph::getNodesSortedById() const {
    std::vector<Node> result;
    result.reserve(nodes.size());
    for (const auto& pair : nodes) {
        result.push_back(pair.second);
    }
    std::sort(result.begin(), result.end(), [](const Node& a, const Node& b) {
        return a.id < b.id;
    });
    return result;
}

bool Graph::setTrafficLevel(int src, int dest, TrafficLevel level, bool bidirectional) {
    bool updated = false;

    // Update src -> dest
    auto itSrc = adj.find(src);
    if (itSrc != adj.end()) {
        for (auto& edge : itSrc->second) {
            if (edge.dest == dest) {
                edge.trafficLevel = level;
                updated = true;
            }
        }
    }

    // Update dest -> src if bidirectional
    if (bidirectional) {
        auto itDest = adj.find(dest);
        if (itDest != adj.end()) {
            for (auto& edge : itDest->second) {
                if (edge.dest == src) {
                    edge.trafficLevel = level;
                    updated = true;
                }
            }
        }
    }

    return updated;
}

bool Graph::setBlocked(int src, int dest, bool blocked, bool bidirectional) {
    bool updated = false;

    // Update src -> dest
    auto itSrc = adj.find(src);
    if (itSrc != adj.end()) {
        for (auto& edge : itSrc->second) {
            if (edge.dest == dest) {
                edge.blocked = blocked;
                updated = true;
            }
        }
    }

    // Update dest -> src if bidirectional
    if (bidirectional) {
        auto itDest = adj.find(dest);
        if (itDest != adj.end()) {
            for (auto& edge : itDest->second) {
                if (edge.dest == src) {
                    edge.blocked = blocked;
                    updated = true;
                }
            }
        }
    }

    return updated;
}

const Edge* Graph::getEdge(int src, int dest) const {
    auto it = adj.find(src);
    if (it != adj.end()) {
        for (const auto& edge : it->second) {
            if (edge.dest == dest) {
                return &edge;
            }
        }
    }
    return nullptr;
}

void Graph::resetTrafficAndBlockages() {
    for (auto& pair : adj) {
        for (auto& edge : pair.second) {
            edge.trafficLevel = TrafficLevel::LOW;
            edge.blocked = false;
        }
    }
}

size_t Graph::getNodeCount() const {
    return nodes.size();
}

size_t Graph::getDirectedEdgeCount() const {
    size_t count = 0;
    for (const auto& pair : adj) {
        count += pair.second.size();
    }
    return count;
}

size_t Graph::getPhysicalRoadCount() const {
    return totalRoadCount;
}

int Graph::getBlockedRoadCount() const {
    int count = 0;
    for (const auto& pair : adj) {
        for (const auto& edge : pair.second) {
            if (edge.src < edge.dest && edge.blocked) {
                count++;
            }
        }
    }
    return count;
}

int Graph::getHighTrafficRoadCount() const {
    int count = 0;
    for (const auto& pair : adj) {
        for (const auto& edge : pair.second) {
            if (edge.src < edge.dest && edge.trafficLevel == TrafficLevel::HIGH) {
                count++;
            }
        }
    }
    return count;
}

std::vector<Edge> Graph::getAllUniqueEdges() const {
    std::vector<Edge> list;
    for (const auto& pair : adj) {
        for (const auto& edge : pair.second) {
            if (edge.src < edge.dest) {
                list.push_back(edge);
            }
        }
    }
    return list;
}

bool Graph::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    nodes.clear();
    adj.clear();
    totalRoadCount = 0;

    std::string line;
    std::string section = "";

    while (std::getline(file, line)) {
        // Strip comments and empty lines
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        while (!line.empty() && (line.back() == ' ' || line.back() == '\r' || line.back() == '\t')) {
            line.pop_back();
        }
        if (line.empty()) continue;

        if (line == "[NODES]") {
            section = "NODES";
            continue;
        } else if (line == "[ROADS]") {
            section = "ROADS";
            continue;
        }

        std::stringstream ss(line);
        if (section == "NODES") {
            int id;
            if (ss >> id) {
                std::string token;
                // Parse CSV formatted: id,name,x,y,category
                std::vector<std::string> parts;
                std::stringstream lineStream(line);
                while (std::getline(lineStream, token, ',')) {
                    // trim
                    size_t start = token.find_first_not_of(" \t");
                    size_t end = token.find_last_not_of(" \t");
                    if (start != std::string::npos && end != std::string::npos) {
                        parts.push_back(token.substr(start, end - start + 1));
                    } else {
                        parts.push_back("");
                    }
                }
                if (parts.size() >= 5) {
                    int nid = std::stoi(parts[0]);
                    std::string nname = parts[1];
                    std::string ntype = parts[2];
                    double nx = std::stod(parts[3]);
                    double ny = std::stod(parts[4]);
                    std::string ndesc = (parts.size() >= 6) ? parts[5] : "";
                    addNode(Node(nid, nname, ntype, nx, ny, ndesc));
                }
            }
        } else if (section == "ROADS") {
            std::vector<std::string> parts;
            std::string token;
            std::stringstream lineStream(line);
            while (std::getline(lineStream, token, ',')) {
                size_t start = token.find_first_not_of(" \t");
                size_t end = token.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos) {
                    parts.push_back(token.substr(start, end - start + 1));
                } else {
                    parts.push_back("");
                }
            }
            if (parts.size() >= 4) {
                int src = std::stoi(parts[0]);
                int dest = std::stoi(parts[1]);
                double dist = std::stod(parts[2]);
                double time = std::stod(parts[3]);
                TrafficLevel tLevel = TrafficLevel::LOW;
                if (parts.size() >= 5) {
                    int tVal = std::stoi(parts[4]);
                    if (tVal == 1) tLevel = TrafficLevel::MEDIUM;
                    else if (tVal == 2) tLevel = TrafficLevel::HIGH;
                }
                addEdge(src, dest, dist, time, tLevel, true);
            }
        }
    }
    return true;
}

void Graph::loadDefaultCampusMap() {
    nodes.clear();
    adj.clear();
    totalRoadCount = 0;

    // 26 Real CGC Landran Campus Locations (Sector 112, Mohali, Punjab)
    addNode(Node(0, "Main Gate 1 (Kharar-Banur Hwy)", "GATE", 5.0, 1.2, "Primary entrance gate on Kharar-Banur Highway with 24/7 security post."));
    addNode(Node(1, "Visitor Parking Plaza", "PARKING", 6.8, 1.5, "Dedicated parking area for campus visitors faculty and official cars."));
    addNode(Node(2, "Admin Block & Corporate Office", "ADMIN", 4.8, 2.5, "Central administrative building housing director office admissions and accounts."));
    addNode(Node(3, "Chandigarh Engineering College (Block 1)", "ACADEMIC", 3.2, 3.8, "Flagship engineering block for mechanical civil and foundational engineering."));
    addNode(Node(4, "CSE & IT Department (Block 2)", "ACADEMIC", 3.0, 5.5, "Computer Science & Engineering complex with AI/ML labs and cloud computing hubs."));
    addNode(Node(5, "Central Library (Knowledge Resource Centre)", "LIBRARY", 4.5, 5.2, "Multi-level central library with extensive digital research resources and reading halls."));
    addNode(Node(6, "Central Lawn & Rose Garden", "GREEN_SPACE", 5.0, 4.0, "Lush central green space with student seating and landscaped paved avenues."));
    addNode(Node(7, "Suraj Parkash Auditorium", "AUDITORIUM", 6.5, 3.5, "1000+ seat air-conditioned main auditorium for convocations and international summits."));
    addNode(Node(8, "Central Canteen & Food Street", "FOOD", 6.5, 5.0, "Multi-vendor food court featuring Nescafe corner fast food and student dining."));
    addNode(Node(9, "Applied Sciences Block (Block 3)", "ACADEMIC", 2.0, 4.5, "First-year engineering classrooms physics/chemistry laboratories and faculty rooms."));
    addNode(Node(10, "Mechanical & Robotics Workshop", "WORKSHOP", 1.8, 6.8, "Practical workshop with CNC machines welding bays and Formula SAE student garage."));
    addNode(Node(11, "Block 4 (Advanced Computing & Labs)", "ACADEMIC", 3.2, 7.2, "Advanced computer labs cyber forensics cell and student innovation incubator."));
    addNode(Node(12, "Block 5 (ECE & Electrical Block)", "ACADEMIC", 4.8, 6.8, "Department of Electronics & Communication with robotics and IoT testbeds."));
    addNode(Node(13, "Chandigarh Business School (CBSA - Block 6)", "ACADEMIC", 7.8, 4.5, "Management studies complex housing MBA BBA and corporate boardroom labs."));
    addNode(Node(14, "College of Pharmacy (CCOP - Block 7)", "ACADEMIC", 8.2, 6.0, "Pharmaceutical sciences building with pharmacology labs and research pilot plant."));
    addNode(Node(15, "Hotel Management Block (CCHM - Block 8)", "ACADEMIC", 7.5, 7.2, "Hospitality training building with demo commercial kitchens and suites."));
    addNode(Node(16, "Boys Hostel Complex (Hostels 1 & 2)", "HOSTEL", 2.5, 8.8, "Residential facility for male students with dining hall gym and warden office."));
    addNode(Node(17, "Girls Hostel Complex (Kalpana Chawla)", "HOSTEL", 7.8, 8.8, "Secure residential complex for female students with recreational rooms and mess."));
    addNode(Node(18, "Campus Sports Stadium & Grounds", "SPORTS", 4.5, 8.8, "Comprehensive outdoor sports facility with cricket ground football pitch and track."));
    addNode(Node(19, "Indoor Gymnasium & Activity Hall", "SPORTS", 5.8, 8.2, "Modern fitness gymnasium table tennis badminton courts and yoga studio."));
    addNode(Node(20, "Campus Dispensary & Health Centre", "MEDICAL", 8.0, 3.0, "24/7 medical room with resident doctor ambulance bay and emergency first-aid."));
    addNode(Node(21, "CGC Transport Yard & Bus Bay", "TRANSIT", 8.5, 1.8, "Fleet terminal for 100+ college buses connecting Chandigarh Mohali and Panchkula."));
    addNode(Node(22, "Student Two-Wheeler Parking", "PARKING", 3.5, 1.5, "Large designated parking zone for student motorcycles scooters and bicycles."));
    addNode(Node(23, "North Gate 2 (Rear Campus Exit)", "GATE", 5.0, 9.5, "Secondary exit gate towards Landran village and staff residential quarters."));
    addNode(Node(24, "Bio-Technology & Research Wing (Block 9)", "ACADEMIC", 6.2, 6.8, "Specialized department for biotechnology genetic engineering and tissue culture."));
    addNode(Node(25, "Open Air Theatre (OAT)", "AMENITY", 5.2, 5.2, "Stepped amphitheatre for cultural performances technical clubs and open discussions."));

    // Interconnecting CGC Campus Pathways & Arterials
    addEdge(0, 1, 0.16, 2.0);
    addEdge(0, 22, 0.13, 1.6);
    addEdge(0, 2, 0.11, 1.4);
    addEdge(1, 7, 0.18, 2.3);
    addEdge(1, 20, 0.15, 1.9);
    addEdge(2, 3, 0.16, 2.0);
    addEdge(2, 6, 0.13, 1.6);
    addEdge(2, 7, 0.18, 2.3);
    addEdge(2, 20, 0.25, 3.1);
    addEdge(3, 4, 0.14, 1.8);
    addEdge(3, 6, 0.15, 1.9);
    addEdge(3, 9, 0.11, 1.4);
    addEdge(3, 22, 0.19, 2.4);
    addEdge(4, 5, 0.13, 1.6);
    addEdge(4, 10, 0.15, 1.9);
    addEdge(4, 11, 0.14, 1.8);
    addEdge(5, 6, 0.10, 1.3);
    addEdge(5, 8, 0.16, 2.0);
    addEdge(5, 12, 0.14, 1.8);
    addEdge(5, 25, 0.07, 0.9);
    addEdge(6, 7, 0.13, 1.6);
    addEdge(6, 8, 0.14, 1.8);
    addEdge(6, 25, 0.10, 1.3);
    addEdge(7, 8, 0.12, 1.5);
    addEdge(7, 13, 0.14, 1.8);
    addEdge(7, 20, 0.14, 1.8);
    addEdge(8, 13, 0.12, 1.5);
    addEdge(8, 15, 0.18, 2.3);
    addEdge(8, 24, 0.15, 1.9);
    addEdge(8, 25, 0.11, 1.4);
    addEdge(9, 10, 0.18, 2.3);
    addEdge(9, 22, 0.25, 3.1);
    addEdge(10, 11, 0.12, 1.5);
    addEdge(11, 12, 0.13, 1.6);
    addEdge(11, 16, 0.15, 1.9);
    addEdge(12, 16, 0.21, 2.6);
    addEdge(12, 18, 0.16, 2.0);
    addEdge(12, 24, 0.11, 1.4);
    addEdge(13, 14, 0.13, 1.6);
    addEdge(13, 20, 0.13, 1.6);
    addEdge(14, 15, 0.12, 1.5);
    addEdge(14, 17, 0.22, 2.8);
    addEdge(14, 21, 0.32, 4.0);
    addEdge(15, 17, 0.14, 1.8);
    addEdge(15, 24, 0.12, 1.5);
    addEdge(16, 18, 0.16, 2.0);
    addEdge(16, 23, 0.21, 2.6);
    addEdge(17, 18, 0.25, 3.1);
    addEdge(17, 19, 0.18, 2.3);
    addEdge(17, 23, 0.23, 2.9);
    addEdge(18, 19, 0.11, 1.4);
    addEdge(18, 23, 0.08, 1.0);
    addEdge(19, 23, 0.12, 1.5);
    addEdge(19, 24, 0.12, 1.5);
    addEdge(20, 21, 0.12, 1.5);
}
