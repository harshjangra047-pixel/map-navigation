# SmartRoute CGC — Intelligent Campus Navigation Using Graph Algorithms
### A Real-World C++ DSA Navigation System for CGC Landran (Sector 112, Mohali, Punjab)

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Web Server](https://img.shields.io/badge/Server-Embedded%20C%2B%2B%20REST%20API-brightgreen.svg)]()
[![Campus](https://img.shields.io/badge/Campus-CGC%20Landran%20Mohali-orange.svg)]()
[![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Linux%20%7C%20Docker-lightgrey.svg)]()
[![Docker](https://img.shields.io/badge/Docker-Ready-blue.svg)]()
[![Deployment](https://img.shields.io/badge/Deploy-Render%20%7C%20Railway%20%7C%20Fly.io-success.svg)]()

> **Educational & Architecture Disclaimer:**
> *Educational DSA prototype. Campus geometry is an approximate digital reconstruction based on publicly available references. SmartRoute CGC is a full-stack Data Structures & Algorithms project built in modern C++ (C++17) with an embedded HTTP REST API backend and a responsive dark futuristic web dashboard. The C++ routing engine executes Dijkstra, A\*, BFS, DFS, and dynamic graph modifications from first principles—there are zero external routing libraries or fake frontend routing scripts.*

---

## 🌐 Quick Start (Run Locally)

```bash
# 1. Navigate to project root
cd "/Users/harsh/Documents/map navigation"

# 2. Build both Web Server & Terminal CLI
./build.sh

# 3. Launch the SmartRoute CGC Web Server
./smartroute_web
# (or simply run: ./run_demo.sh)
```

Open your browser at:
### 👉 **[http://localhost:8080](http://localhost:8080)**

```
========================================
SMARTROUTE WEB SERVER
Server started successfully
Open:
http://localhost:8080
========================================
```

---

## 🏛️ Real Campus Reference & Topology

- **Institution:** Chandigarh Group of Colleges (CGC) Landran
- **Location:** Sector 112, Landran, Mohali, Punjab 140307, India
- **Coordinates:** 30.6865° N, 76.6647° E
- **Graph Topology:** 26 campus landmarks & 55 interconnecting pathways
- **Graph Representation:** Adjacency List (`std::unordered_map<int, std::vector<Edge>>`)

### Verified Landmark Categories
- **Main Entrance:** Main Gate 1 on Kharar-Banur Highway (NH 205A), North Gate 2 (Rear Campus Exit)
- **Central Facilities:** Admin Block & Corporate Office, Central Library (Knowledge Resource Centre), Central Lawn & Rose Garden, Open Air Theatre (OAT)
- **Academic Complexes:** CEC Block 1 (Chandigarh Engineering College), Block 2 (CSE & IT Department), Block 3 (Applied Sciences), Block 4 (Advanced Computing), Block 5 (ECE & Electrical), CBSA Block 6 (Chandigarh Business School), CCOP Block 7 (Pharmacy), CCHM Block 8 (Hotel Management), Block 9 (Biotechnology & Research)
- **Student Life & Dining:** Central Canteen & Food Street (Nescafe Court), Suraj Parkash Auditorium (1000+ seat)
- **Hostel Accommodations:** Boys Hostel Complex (Hostels 1 & 2), Girls Hostel Complex (Kalpana Chawla)
- **Athletics & Health:** Campus Sports Stadium & Cricket Ground, Indoor Gymnasium & Activity Hall, Campus Dispensary & Health Centre
- **Logistics & Parking:** Visitor Parking Plaza, Student Two-Wheeler Parking, CGC Transport Yard & Bus Bay, Mechanical & Robotics Workshop

---

## 🏗️ System Architecture

```
                                  BROWSER (HTML5 / CSS3 / Vanilla JS)
                                                 │
                                                 │ HTTP Requests / JSON
                                                 ▼
                             C++ REST API WEB SERVER (cpp-httplib)
                                                 │
                                                 │ Thread-Safe Mutex Lock
                                                 ▼
                              C++ SMARTROUTE NAVIGATION ENGINE
                                                 │
               ┌─────────────────────────────────┼─────────────────────────────────┐
               ▼                                 ▼                                 ▼
      GRAPH ADJACENCY LIST              DIJKSTRA / A* SEARCH              TRAFFIC & BLOCKAGES
   std::unordered_map<int,              Min-Heap Priority Queue          Dynamic Edge Weight Multipliers
   std::vector<Edge>> & Nodes          Euclidean Distance Heuristic      (LOW: 1.0x, MED: 1.5x, HIGH: 2.5x)
```

---

## 🚀 Key Features

1. **Embedded C++ Web Server:** Powered by `cpp-httplib` and `nlohmann/json` vendored into `third_party/`—zero heavyweight framework dependencies.
2. **Interactive SVG Campus Digital Twin:** Visualizes the 26 CGC Landran landmarks with 3D isometric building footprints, green lawns, sports stadium oval, and real pathway geometries.
3. **Dijkstra vs. A\* Live Benchmarking Lab:** Real-time side-by-side comparison modal displaying nodes explored, actual C++ CPU execution time in milliseconds, and proof of Euclidean heuristic admissibility.
4. **Simulated Campus Traffic:** Set any pathway to `LOW` ($1.0\times$), `MEDIUM` ($1.5\times$), or `HIGH` ($2.5\times$) congestion. The C++ engine dynamically reroutes around peak student rushes.
5. **Dynamic Road Closures:** Close any pathway with one click; the engine automatically recomputes an optimal bypass.
6. **Alternative Corridor Routing:** Computes secondary bypass corridors with automated rationale.
7. **Graph Traversals (BFS & DFS):** Live animated node expansion on the campus map comparing queue-based level order vs stack-based depth exploration.
8. **Simulated Live Navigation Mode:** Heads-up display (HUD) showing "You Are Here", next waypoint, remaining distance, and animated walking marker.
9. **DSA Graph Inspector Modal:** Runtime priority queue states, $f(n) = g(n) + h(n)$ breakdowns, and vertex lists.
10. **Viva & Presentation Guide:** Detailed college examination cheatsheet.

---

## 📡 REST API Reference

| Method | Endpoint | Description | Sample Request / Response |
| :--- | :--- | :--- | :--- |
| `GET` | `/api/health` | Service health check | `{"status": "ok", "service": "SmartRoute CGC", "engine": "C++"}` |
| `GET` | `/api/campus` | Full campus graph topology | 26 nodes, 55 pathways, distances, walking times, traffic states |
| `GET` | `/api/locations` | All 26 campus landmarks | `[{"id": 0, "name": "Main Gate 1", "type": "GATE", "x": 5.0, "y": 1.2}, ...]` |
| `POST` | `/api/route` | Compute optimal route | **Req:** `{"source": "Main Gate 1", "destination": "CSE & IT Department", "algorithm": "astar", "mode": "walking"}`<br>**Res:** `{"success": true, "algorithm": "A*", "route": [...], "distance_km": 0.41, "travel_time_min": 5.2, "nodes_explored": 11, "execution_time_ms": 0.01}` |
| `POST` | `/api/traffic` | Set pathway congestion | **Req:** `{"from": 2, "to": 6, "level": "HIGH"}` |
| `POST` | `/api/road/block` | Close pathway segment | **Req:** `{"from": 0, "to": 2}` |
| `POST` | `/api/road/unblock` | Reopen pathway segment | **Req:** `{"from": 0, "to": 2}` |
| `POST` | `/api/traversal` | Run BFS or DFS | **Req:** `{"start_node": 0, "algorithm": "bfs"}` |
| `GET` | `/api/statistics` | System & graph stats | `{"total_locations": 26, "total_roads": 55, "blocked_roads": 0, ...}` |
| `POST` | `/api/reset` | Clear blockages & traffic | `{"success": true, "message": "Graph network successfully reset"}` |
| `GET` | `/api/algorithm-info` | Theoretical DSA breakdown | Formatted documentation for Dijkstra, A*, BFS, DFS, and Adjacency List |

---

## 📊 DSA Complexity Analysis

| Component / Algorithm | Data Structure | Time Complexity | Space Complexity |
| :--- | :--- | :--- | :--- |
| **Graph Representation** | `std::unordered_map<int, std::vector<Edge>>` | $\mathcal{O}(\text{deg}(u))$ neighbor lookup | $\mathcal{O}(V + E)$ |
| **Dijkstra's Algorithm** | `std::priority_queue` (Min-Heap) | $\mathcal{O}((V + E) \log V)$ | $\mathcal{O}(V)$ |
| **A\* Search Algorithm** | `std::priority_queue` (Min-Heap) | $\mathcal{O}(E)$ best $\rightarrow \mathcal{O}((V + E) \log V)$ worst | $\mathcal{O}(V)$ |
| **BFS Traversal** | `std::queue<int>` (FIFO) | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ |
| **DFS Traversal** | `std::vector<int>` (LIFO Stack) | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ |

---

## 🚢 Deploying Publicly (Render / Docker)

### Deploy to Render
1. Push this project to GitHub:
   ```bash
   git init
   git add .
   git commit -m "SmartRoute CGC: C++ Full-Stack Campus Navigation"
   git branch -M main
   git remote add origin https://github.com/<your-username>/smartroute-cgc.git
   git push -u origin main
   ```
2. Log in to [Render.com](https://render.com) and click **New +** $\rightarrow$ **Web Service**.
3. Select your GitHub repository.
4. Set the configuration:
   - **Environment:** `Docker`
   - **Branch:** `main`
   - **Plan:** Free
   - **Health Check Path:** `/api/health`
5. Click **Create Web Service**. Render builds the `Dockerfile` and deploys on a public URL.

### Run with Local Docker
```bash
docker build -t smartroute-cgc .
docker run -p 8080:8080 -e PORT=8080 smartroute-cgc
```

---

## 🧪 Verification & Unit Testing

```bash
make test
```
All 8 test suites verify:
- CGC Landran 26 landmarks and 55 pathways loaded
- Source == Destination edge cases
- Dijkstra and A* mathematical agreement
- Dynamic traffic route shifting
- Road blockage bypass recalculation
- Isolated / disconnected network handling
- BFS & DFS complete campus reachability
- Alternative corridor discovery

---

## 📜 Educational Safety & Accuracy Notice
*SmartRoute CGC is an educational academic prototype designed for demonstrating Graph Algorithms and Data Structures. Campus geometry is an approximate digital reconstruction based on publicly available references and maps. It is not an official municipal or university surveying system.*
