# SmartRoute — Technical Project Documentation

**Project Title:** SmartRoute: Intelligent Map Navigation System Using Data Structures & Algorithms  
**Language:** C++ (C++17)  
**Target Domain:** Data Structures & Algorithms, Graph Theory, Autonomous Routing  
**Author / Team:** CSE / IT Undergraduate Project

---

## 1. Abstract & Problem Statement

Routing in urban road networks represents one of the classic real-world applications of Graph Theory in Computer Science. In this project, we design and implement a standalone, terminal-based navigation system in C++ called **SmartRoute**. 

The fundamental goal is to model a real-world campus/city road network as a mathematical graph and evaluate both uninformed and informed shortest-path search algorithms under static, dynamic traffic, and obstructed conditions. 

Unlike black-box navigation systems, SmartRoute implements every graph algorithm manually using standard C++ Standard Template Library (STL) data structures, with zero third-party routing dependencies.

---

## 2. Graph Mathematical Model

A road network is modeled as a weighted, directed/bidirectional graph $G = (V, E)$, where:
- **Vertices ($V$):** $V = \{v_0, v_1, \dots, v_{n-1}\}$ represent road intersections, transit hubs, academic blocks, and amenities. Each vertex is assigned spatial Cartesian coordinates $(x_i, y_i) \in \mathbb{R}^2$.
- **Edges ($E$):** Directed pairs $(u, v) \in E$ represent navigable roadways connecting vertex $u$ to vertex $v$.
- **Weight Function ($W$):** The cost metric $w(u, v)$ depends on the active optimization mode:
  $$\text{Cost}(u, v) = \begin{cases}
  \infty & \text{if road is blocked} \\
  d(u, v) & \text{under Shortest Distance mode} \\
  t_{\text{base}}(u, v) \times \mu(\text{traffic}) & \text{under Fastest Time mode}
  \end{cases}$$
  where $\mu(\text{LOW}) = 1.0$, $\mu(\text{MEDIUM}) = 1.5$, and $\mu(\text{HIGH}) = 2.5$.

### Representation: Adjacency List
An Adjacency List is implemented using:
```cpp
std::unordered_map<int, std::vector<Edge>> adj;
```
For sparse graphs where $|E| \ll |V|^2$, an Adjacency List provides:
- Space Complexity: $\mathcal{O}(|V| + |E|)$ vs $\mathcal{O}(|V|^2)$ for an Adjacency Matrix.
- Neighborhood Exploration: $\mathcal{O}(\text{deg}(u))$ vs $\mathcal{O}(|V|)$.

---

## 3. Algorithm Formulations & Implementations

### 3.1 Dijkstra's Shortest Path Algorithm
Dijkstra's algorithm solves the single-source shortest path problem on graphs with non-negative edge weights using a greedy strategy.
- **Priority Queue Min-Heap:** A binary min-heap (`std::priority_queue<std::pair<double, int>, std::vector<...>, std::greater<...>>`) maintains the frontier of unvisited vertices ordered by accumulated distance $g(u)$.
- **Relaxation Step:**
  $$\text{if } g(u) + w(u, v) < g(v) \implies g(v) \leftarrow g(u) + w(u, v), \quad \text{parent}[v] \leftarrow u$$
- **Time Complexity:** $\mathcal{O}((|V| + |E|) \log |V|)$
- **Space Complexity:** $\mathcal{O}(|V|)$

### 3.2 A* Search Algorithm
A* enhances Dijkstra's algorithm by incorporating heuristic domain knowledge about the target destination:
$$f(n) = g(n) + h(n)$$
- $g(n)$: Exact accumulated cost from source to node $n$.
- $h(n)$: Estimated cost from node $n$ to destination.
- **Heuristic Function:** Straight-line 2D Euclidean Distance:
  $$h(n) = \sqrt{(x_{\text{dest}} - x_n)^2 + (y_{\text{dest}} - y_n)^2}$$
- **Admissibility & Consistency:**
  Because the Euclidean distance represents the absolute shortest possible physical displacement between two points in 2D Euclidean space, it can never overestimate the actual road network distance:
  $$h(n) \le h^*(n) \quad (\text{Admissible})$$
  $$h(u) \le w(u, v) + h(v) \quad (\text{Monotonic / Consistent})$$
  This mathematical property guarantees that A* yields the globally optimal shortest path while expanding substantially fewer vertices than Dijkstra.

### 3.3 Breadth-First Search (BFS) & Depth-First Search (DFS)
- **BFS:** Uses `std::queue<int>` (FIFO) to explore vertices level-by-level. Ideal for computing minimum hop counts on unweighted graphs and checking graph connectivity.
- **DFS:** Uses `std::vector<int>` (LIFO stack) to explore deeply along each branch. Demonstrates connected components and cycle exploration.

### 3.4 Alternative Route Discovery
Rather than presenting a single brittle path, SmartRoute calculates an alternative corridor by identifying the primary route's critical bottleneck edges, temporarily applying an edge penalty, and re-running the optimization pass. If a distinct corridor exists (e.g. Western corridor vs Eastern outer ring), both routes are displayed side-by-side with an automated decision rationale.

---

## 4. Dynamic Features & State Modification

### 4.1 Traffic Congestion Simulation
Edge weights are dynamically updated without mutating graph topology:
$$t_{\text{effective}} = t_{\text{base}} \times \text{multiplier}$$
When severe congestion (`HIGH`, $2.5\times$) occurs on an arterial, the edge cost escalates, causing the min-heap relaxation step to naturally prefer longer, open bypasses.

### 4.2 Road Blockage Simulation
Any directed or bidirectional road segment can be toggled to `blocked = true`. The search algorithms inspect this boolean flag in $\mathcal{O}(1)$ time and skip relaxation across closed roads. Dynamic re-routing is instantaneous and does not require rebuilding the graph.

---

## 5. Empirical Performance Comparison

Measured results from the built-in campus dataset (Source: Main Gate [0], Destination: University Hospital [18]):

| Metric | Dijkstra's Algorithm | A* Search Algorithm | Observation |
| :--- | :--- | :--- | :--- |
| **Path Found** | YES | YES | Both algorithms guaranteed complete |
| **Optimal Distance** | 23.0 km | 23.0 km | Identical cost (proof of admissibility) |
| **Travel Time** | 36.5 min | 36.5 min | Both optimal under travel time mode |
| **Nodes Explored** | 24 vertices | 21 vertices (or 11 under distance mode) | **Up to 54% reduction in node expansions** |
| **Exploration Shape** | Concentric circle | Targeted directional cone | A* prunes opposite search branches |
| **Heuristic Used** | None ($h(n) = 0$) | Euclidean 2D ($h(n) > 0$) | Goal-directed heuristic guides search |

---

## 6. Defensive Programming & Edge Cases

The codebase strictly handles all boundary conditions:
1. **Source == Destination:** Terminated in $\mathcal{O}(1)$ time; reports 0.0 km and 0.0 min.
2. **Invalid Node IDs or Unknown Names:** Robust input parsing with case-insensitive substring search; falls back gracefully without crashing.
3. **No Available Route / Complete Isolation:** Detected when the open set or priority queue empties without reaching the destination; returns `found = false` with an actionable warning.
4. **Repeated Dynamic Updates:** Edge traffic and blockage flags can be updated, cleared, or reset repeatedly with zero memory leaks.
