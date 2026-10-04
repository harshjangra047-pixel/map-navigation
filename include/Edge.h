#ifndef SMARTROUTE_EDGE_H
#define SMARTROUTE_EDGE_H

#include <string>

/**
 * @brief Represents traffic congestion levels on a road segment.
 * Each level modifies the effective travel time via a calibrated multiplier.
 */
enum class TrafficLevel {
    LOW = 0,     // Normal traffic: 1.0x multiplier
    MEDIUM = 1,  // Moderate traffic: 1.5x multiplier
    HIGH = 2     // Severe congestion: 2.5x multiplier
};

/**
 * @brief Route calculation modes for cost optimization.
 */
enum class RouteMode {
    SHORTEST_DISTANCE = 1, // Minimize physical road distance (km)
    FASTEST_TIME = 2,      // Minimize effective travel time (minutes, considering traffic)
    AVOID_HIGH_TRAFFIC = 3,// Penalize high-traffic roads to seek smoother corridors
    EMERGENCY = 4          // Emergency priority: strictly fastest open arterial with heavy traffic penalty
};

/**
 * @brief Represents a directed road connection between two vertices.
 * In a bidirectional road, two directed edges are added (u -> v and v -> u).
 */
struct Edge {
    int src;                   // Source vertex ID
    int dest;                  // Destination vertex ID
    double distance;           // Physical distance (in kilometers)
    double baseTravelTime;     // Base travel time under clear conditions (in minutes)
    TrafficLevel trafficLevel; // Current simulated congestion status
    bool blocked;              // True if the road is closed/blocked

    Edge() 
        : src(-1), dest(-1), distance(0.0), baseTravelTime(0.0), 
          trafficLevel(TrafficLevel::LOW), blocked(false) {}

    Edge(int src, int dest, double distance, double baseTravelTime, 
         TrafficLevel trafficLevel = TrafficLevel::LOW, bool blocked = false)
        : src(src), dest(dest), distance(distance), baseTravelTime(baseTravelTime),
          trafficLevel(trafficLevel), blocked(blocked) {}

    /**
     * @brief Gets congestion multiplier based on traffic level.
     */
    double getTrafficMultiplier() const {
        switch (trafficLevel) {
            case TrafficLevel::LOW:    return 1.0;
            case TrafficLevel::MEDIUM: return 1.5;
            case TrafficLevel::HIGH:   return 2.5;
            default:                   return 1.0;
        }
    }

    /**
     * @brief Effective travel time accounting for traffic congestion.
     */
    double getEffectiveTravelTime() const {
        return baseTravelTime * getTrafficMultiplier();
    }

    /**
     * @brief Computes edge weight dynamically based on RouteMode.
     * Returns a large number (infinity) if blocked.
     */
    double getCost(RouteMode mode) const {
        if (blocked) return 1e18; // Effectively infinite / blocked

        switch (mode) {
            case RouteMode::SHORTEST_DISTANCE:
                return distance;

            case RouteMode::FASTEST_TIME:
                return getEffectiveTravelTime();

            case RouteMode::AVOID_HIGH_TRAFFIC: {
                double time = getEffectiveTravelTime();
                if (trafficLevel == TrafficLevel::HIGH) {
                    return time * 2.0; // Strong penalty for HIGH traffic
                } else if (trafficLevel == TrafficLevel::MEDIUM) {
                    return time * 1.2;
                }
                return time;
            }

            case RouteMode::EMERGENCY: {
                // Emergency vehicles prioritize high speed and low choke points
                double time = getEffectiveTravelTime();
                if (trafficLevel == TrafficLevel::HIGH) {
                    return time * 3.0; // Severe congestion heavily avoided
                }
                return time;
            }

            default:
                return distance;
        }
    }

    static std::string trafficLevelToString(TrafficLevel level) {
        switch (level) {
            case TrafficLevel::LOW:    return "LOW (1.0x)";
            case TrafficLevel::MEDIUM: return "MEDIUM (1.5x)";
            case TrafficLevel::HIGH:   return "HIGH (2.5x)";
            default:                   return "UNKNOWN";
        }
    }
};

#endif // SMARTROUTE_EDGE_H
