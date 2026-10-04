#!/bin/bash
# ==============================================================================
# SmartRoute Build Script
# Compiles both SmartRoute Terminal App and Web Server
# ==============================================================================

set -e

echo "=========================================================="
echo "    Building SmartRoute C++ Engine & Web Application      "
echo "=========================================================="

CXX="clang++"
if ! command -v clang++ &> /dev/null; then
    if command -v g++ &> /dev/null; then
        CXX="g++"
    else
        echo "Error: No suitable C++ compiler (clang++ or g++) found."
        exit 1
    fi
fi

echo "[1/3] Using C++ Compiler: $CXX"

echo "[2/3] Compiling SmartRoute Terminal CLI..."
$CXX -std=c++17 -Wall -Wextra -O2 -Iinclude \
    src/main.cpp \
    src/UI.cpp \
    src/Graph.cpp \
    src/Algorithms.cpp \
    src/Navigation.cpp \
    -o smartroute

echo "[3/3] Compiling SmartRoute Web Server..."
$CXX -std=c++17 -Wall -Wextra -O2 -Iinclude -Ithird_party \
    src/server_main.cpp \
    src/Server.cpp \
    src/Graph.cpp \
    src/Algorithms.cpp \
    src/Navigation.cpp \
    -pthread \
    -o smartroute_web

echo ""
echo "=========================================================="
echo " BUILD SUCCESSFUL!"
echo "   1. Web Server:     ./smartroute_web   (http://localhost:8080)"
echo "   2. Terminal CLI:   ./smartroute       (or ./run_demo.sh)"
echo "=========================================================="
