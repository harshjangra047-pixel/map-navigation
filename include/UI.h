#ifndef SMARTROUTE_UI_H
#define SMARTROUTE_UI_H

#include "Navigation.h"
#include <string>

/**
 * @brief Terminal User Interface & Map Visualizer.
 * Provides interactive menus, ANSI styling, ASCII network maps,
 * route trace visualizers, and educational viva explanations.
 */
class UI {
private:
    Navigation& nav;

    // ANSI Color Helpers
    static const std::string RESET;
    static const std::string BOLD;
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string MAGENTA;
    static const std::string CYAN;
    static const std::string WHITE;
    static const std::string BG_BLUE;
    static const std::string BG_DARK;

public:
    explicit UI(Navigation& navigation);

    // Main Interactive Loop
    void run();

    // Menu Screens
    void displayHeader();
    void displayMainMenu();
    void handleFindRoute();
    void handleCompareAlgorithms();
    void handleAlternativeRoute();
    void handleSimulateTraffic();
    void handleBlockRoad();
    void handleTraversals();
    void handleViewMap();
    void handleVivaMode();
    void handleProjectStatistics();
    void handleRunPresentationDemo();
    void handleResetMap();

    // Visualizers & Formatters
    void printRouteResult(const SearchResult& result, bool showStepByStep = true);
    void printComparisonTable(const SearchResult& dijkstraRes, const SearchResult& aStarRes);
    void renderAsciiMap(const std::vector<int>& highlightedPath = {});
    void printNodeDirectory();
    void printRoadDirectory();

    // Robust Input Handlers
    int getValidInt(const std::string& prompt, int minVal, int maxVal);
    int promptForLocation(const std::string& role);
    void pauseForUser();
    void clearScreen();
};

#endif // SMARTROUTE_UI_H
