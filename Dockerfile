# ==============================================================================
# SmartRoute Web Server — Dockerfile
# Production-ready containerized build for Render, Railway, Fly.io, or local Docker
# ==============================================================================

# Build & Runtime Stage
FROM ubuntu:22.04

# Avoid interactive prompts during apt installs
ENV DEBIAN_FRONTEND=noninteractive

# Install modern C++ compiler, build tools, and utilities
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    g++ \
    cmake \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Set working directory inside container
WORKDIR /app

# Copy full project source files
COPY . .

# Compile both CLI and Web Server with high optimization
RUN g++ -std=c++17 -Wall -Wextra -O2 -Iinclude -Ithird_party \
    src/Graph.cpp \
    src/Algorithms.cpp \
    src/Navigation.cpp \
    src/Server.cpp \
    src/server_main.cpp \
    -pthread \
    -o smartroute_web

RUN g++ -std=c++17 -Wall -Wextra -O2 -Iinclude \
    src/Graph.cpp \
    src/Algorithms.cpp \
    src/Navigation.cpp \
    src/UI.cpp \
    src/main.cpp \
    -o smartroute

# Expose default HTTP port
EXPOSE 8080

# Cloud platforms like Render supply $PORT dynamically
ENV PORT=8080

# Start SmartRoute Web Server
CMD ["./smartroute_web"]
