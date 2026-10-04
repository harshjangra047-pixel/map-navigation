#include "Navigation.h"
#include "UI.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // Instantiate navigation engine
    Navigation navigation;

    // Check if custom data path provided via CLI
    std::string dataPath = "data/city_map.txt";
    if (argc > 1 && argv[1][0] != '-') {
        dataPath = argv[1];
    }

    if (!navigation.initialize(dataPath)) {
        std::cerr << "Warning: Could not open dataset file at " << dataPath 
                  << ". Using built-in 24-node campus dataset.\n";
    }

    // Launch UI
    UI ui(navigation);

    // Support quick-run demo flag for presentation convenience
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--demo" || arg == "-d") {
            ui.handleRunPresentationDemo();
            return 0;
        } else if (arg == "--viva" || arg == "-v") {
            ui.handleVivaMode();
            return 0;
        } else if (arg == "--map" || arg == "-m") {
            ui.handleViewMap();
            return 0;
        }
    }

    // Standard interactive execution
    ui.run();

    return 0;
}
