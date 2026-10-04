#ifndef SMARTROUTE_NODE_H
#define SMARTROUTE_NODE_H

#include <string>
#include <cmath>

/**
 * @brief Represents a campus landmark or intersection on the CGC Landran campus.
 * 
 * DSA Concept: Vertex in an Adjacency List Graph representation.
 * Stores 2D spatial Cartesian coordinates (x, y) calibrated to CGC Landran layout.
 * Used by A* Euclidean distance heuristic h(n) = sqrt((x2 - x1)^2 + (y2 - y1)^2).
 */
struct Node {
    int id;                 // Unique identifier for the landmark (0 to V-1)
    std::string name;       // Landmark name (e.g. "CSE & IT Department (Block 2)")
    std::string type;       // Type: GATE, ACADEMIC, LIBRARY, HOSTEL, FOOD, SPORTS, MEDICAL, AUDITORIUM, PARKING, ADMIN, GREEN_SPACE, WORKSHOP
    double x;               // Spatial Cartesian X coordinate (0.0 to 10.0 scale)
    double y;               // Spatial Cartesian Y coordinate (0.0 to 10.0 scale)
    std::string description;// Educational & practical context for the landmark

    Node() 
        : id(-1), name(""), type("ACADEMIC"), x(0.0), y(0.0), description("") {}

    Node(int id, const std::string& name, const std::string& type, double x, double y, const std::string& description = "")
        : id(id), name(name), type(type), x(x), y(y), description(description) {}

    /**
     * @brief Computes Euclidean straight-line distance to another campus node.
     * Admissible and consistent heuristic h(n) in 2D metric space.
     */
    double euclideanDistanceTo(const Node& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

#endif // SMARTROUTE_NODE_H
