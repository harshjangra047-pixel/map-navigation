#ifndef SMARTROUTE_SERVER_H
#define SMARTROUTE_SERVER_H

#include "Navigation.h"
#include <string>
#include <memory>
#include <mutex>

namespace httplib {
    class Server;
}

/**
 * @brief Embedded C++ HTTP REST API Web Server for SmartRoute.
 * 
 * Uses cpp-httplib and nlohmann::json to serve the web application frontend
 * and provide real-time REST API endpoints connecting to the C++ DSA routing engine.
 */
class Server {
private:
    Navigation& nav;
    std::mutex navMutex; // Thread safety for concurrent API requests
    std::unique_ptr<httplib::Server> httpServer;
    std::string frontendDir;

    // Route Registration
    void registerRoutes();

    // Helper parsers
    int resolveNode(const std::string& input);

public:
    explicit Server(Navigation& navigation, const std::string& staticDir = "./frontend");
    ~Server();

    // Start listening
    bool start(int port = 8080, const std::string& host = "0.0.0.0");
    void stop();
};

#endif // SMARTROUTE_SERVER_H
