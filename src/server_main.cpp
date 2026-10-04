#include "Navigation.h"
#include "Server.h"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Determine Port from environment variable PORT (required for Render / cloud deployment)
    int port = 8080;
    const char* envPort = std::getenv("PORT");
    if (envPort) {
        try {
            port = std::stoi(envPort);
        } catch (...) {
            port = 8080;
        }
    } else if (argc > 1 && argv[1][0] != '-') {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    // Determine Static frontend directory
    std::string frontendDir = "./frontend";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--static" && i + 1 < argc) {
            frontendDir = argv[++i];
        }
    }

    // Initialize Navigation DSA engine
    Navigation nav;
    std::string dataPath = "data/city_map.txt";
    if (!nav.initialize(dataPath)) {
        std::cerr << "Warning: Could not open dataset at " << dataPath 
                  << ". Using built-in 24-node campus dataset.\n";
    }

    // Create and launch web server
    Server server(nav, frontendDir);
    if (!server.start(port, "0.0.0.0")) {
        std::cerr << "Error: Could not bind to port " << port << ".\n";
        return 1;
    }

    return 0;
}
