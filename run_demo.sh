#!/bin/bash
# ==============================================================================
# SmartRoute CGC — Presentation & Web Application Launcher
# ==============================================================================

if [ "$1" == "--cli" ] || [ "$1" == "-c" ]; then
    if [ ! -f ./smartroute ]; then
        echo "Building SmartRoute CLI..."
        ./build.sh
    fi
    ./smartroute --demo
else
    if [ ! -f ./smartroute_web ]; then
        echo "Building SmartRoute Web Server..."
        ./build.sh
    fi
    echo "Starting SmartRoute CGC Campus Navigation Web Server..."
    ./smartroute_web
fi
