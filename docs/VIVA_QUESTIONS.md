# SmartRoute — Comprehensive Viva Questions & Model Answers

This document prepares students for external project examinations, viva-voce evaluations, and technical interviews.

---

### Category 1: Graph Representation & Data Structures

#### Q1: Why is a Graph data structure required to represent a road network?
**Answer:** A road network consists of arbitrary intersections (vertices) connected by roads (edges) that may contain cycles, alternative corridors, and varying weights. Linear data structures (arrays, linked lists) and hierarchical structures (trees) cannot represent cycles or multiple intersecting pathways. Hence, a Graph $G = (V, E)$ is the only mathematically sound representation.

#### Q2: Why did you choose an Adjacency List over an Adjacency Matrix?
**Answer:** Real-world road networks are **sparse graphs**. In our 24-node map, the average node degree is ~3.75 roads per intersection.
- **Space Complexity:** Adjacency List takes $\mathcal{O}(V + E)$ space (~69 elements), whereas an Adjacency Matrix takes $\mathcal{O}(V^2) = 576$ cells, wasting significant memory on zeros.
- **Time Complexity:** Finding all neighbors of vertex $u$ in an Adjacency List takes $\mathcal{O}(\text{deg}(u))$ time, whereas in an Adjacency Matrix it requires an $\mathcal{O}(V)$ scan over all columns regardless of how few roads connect to $u$.

#### Q3: Which C++ STL containers did you use to build the Adjacency List?
**Answer:** 
- `std::unordered_map<int, Node>`: Provides $\mathcal{O}(1)$ average-time lookup for node metadata (name, coordinates, category).
- `std::unordered_map<int, std::vector<Edge>>`: Maps each node ID to a dynamic contiguous array (`std::vector`) of outgoing `Edge` structures.
- `std::priority_queue<std::pair<double, int>, std::vector<...>, std::greater<...>>`: Implements a binary min-heap for the frontier in Dijkstra and A*.

#### Q4: Why is road connectivity bidirectional in this project?
**Answer:** Most urban roadways accommodate traffic in both directions. In our implementation, a two-way road between node $u$ and node $v$ is modeled as two directed edges: $(u \rightarrow v)$ and $(v \rightarrow u)$. This preserves the flexibility to simulate one-way streets or asymmetric congestion.

---

### Category 2: Dijkstra's Algorithm

#### Q5: What is Dijkstra's algorithm and what problem does it solve?
**Answer:** Dijkstra's algorithm is a greedy algorithm that solves the Single-Source Shortest Path (SSSP) problem on a weighted graph with non-negative edge weights. It finds the minimal cost path from a designated source vertex to all other vertices (or terminates early upon settling the target vertex).

#### Q6: How does the priority queue optimize Dijkstra's algorithm?
**Answer:** 
- Without a priority queue (using a simple array), finding the unvisited vertex with the minimum tentative distance takes linear search: $\mathcal{O}(V)$ per iteration, resulting in total time $\mathcal{O}(V^2)$.
- Using a binary min-heap (`std::priority_queue`), extracting the minimum takes $\mathcal{O}(\log V)$ and edge relaxation takes $\mathcal{O}(\log V)$, reducing overall time complexity to $\mathcal{O}((V + E) \log V)$.

#### Q7: Can Dijkstra's algorithm handle negative edge weights?
**Answer:** No. Dijkstra relies on the greedy property that once a node is extracted from the min-heap (settled), its shortest path distance is finalized and cannot be improved. If negative weights exist, a longer path with a negative edge could later reduce the cost of a settled node, invalidating the algorithm. To handle negative weights, the Bellman-Ford algorithm ($\mathcal{O}(V \cdot E)$) must be used.

#### Q8: How do you reconstruct the actual path after Dijkstra finishes?
**Answer:** We maintain a predecessor map: `std::unordered_map<int, int> parent`. Whenever relaxation updates $g(v) = g(u) + w(u, v)$, we record `parent[v] = u`. Once destination $d$ is reached, we trace backwards: $d \rightarrow \text{parent}[d] \rightarrow \text{parent}[\text{parent}[d]] \dots \rightarrow s$, and reverse the sequence.

---

### Category 3: A* Search Algorithm & Heuristics

#### Q9: What is A* search and how does it differ from Dijkstra?
**Answer:** A* is an **informed (heuristic) search algorithm**. While Dijkstra evaluates nodes purely based on known cost from the start $g(n)$, A* evaluates nodes using:
$$f(n) = g(n) + h(n)$$
where $h(n)$ is an estimated cost from node $n$ to the goal. This heuristic steers the search directly toward the target, pruning unpromising branches that point away from the goal.

#### Q10: What heuristic function did you use, and why?
**Answer:** We used **2D Euclidean Distance (straight-line distance)**:
$$h(n) = \sqrt{(x_{\text{dest}} - x_n)^2 + (y_{\text{dest}} - y_n)^2}$$
In 2D physical space, the straight line between two points is the shortest possible distance (triangle inequality). Therefore, real road distance is always $\ge$ straight-line distance.

#### Q11: What does it mean for a heuristic to be "Admissible"?
**Answer:** An admissible heuristic never overestimates the true minimal cost to reach the goal:
$$h(n) \le h^*(n) \quad \forall n$$
If a heuristic were to overestimate the cost, A* might prematurely dismiss the true optimal path thinking it was too expensive. Admissibility guarantees that A* finds the optimal shortest path.

#### Q12: What is "Consistency" (or Monotonicity) in A*?
**Answer:** A heuristic is consistent if for every node $u$ and every neighbor $v$:
$$h(u) \le w(u, v) + h(v)$$
and $h(\text{goal}) = 0$. Consistency is the triangle inequality applied to heuristics. It ensures that whenever A* settles a node, the path to that node is optimal, meaning nodes never need to be reopened. Euclidean distance in 2D metric space is naturally consistent.

#### Q13: Is A* always faster than Dijkstra?
**Answer:** No, not in raw CPU time per step. A* computes heuristic evaluations and maintains additional tracking, which adds minor constant-factor overhead. However, on large graphs, A* explores significantly fewer nodes (often 40%–80% fewer), making total elapsed time substantially lower. If $h(n) = 0$ everywhere, A* degrades exactly into Dijkstra's algorithm.

---

### Category 4: Graph Traversals (BFS & DFS)

#### Q14: Why can BFS NOT replace Dijkstra for finding shortest paths in map navigation?
**Answer:** Standard BFS assumes all edges have uniform, unit weight ($w=1$). It finds the path with the **minimum number of edges (hops)**. In real road networks, roads have differing lengths and travel times. A 2-hop route could be 20 km long, while an alternative 3-hop route might be only 5 km. BFS would select the 2-hop path, which is suboptimal.

#### Q15: What are the primary uses of BFS and DFS in this project?
**Answer:** 
- **BFS:** Used to demonstrate level-order exploration and verify unweighted reachability (connected components).
- **DFS:** Used to traverse deep acyclic branches and demonstrate stack-based graph exploration.

#### Q16: What data structures do BFS and DFS use?
**Answer:**
- BFS uses a **FIFO Queue** (`std::queue<int>`) to visit nodes in order of distance from the root.
- DFS uses a **LIFO Stack** (`std::vector<int>` or call stack recursion) to explore as deep as possible before backtracking.

---

### Category 5: Dynamic Features & Edge Cases

#### Q17: How is dynamic traffic simulated without external APIs?
**Answer:** Each road edge maintains a `TrafficLevel` enum (`LOW`, `MEDIUM`, `HIGH`). When traffic increases:
$$\text{effective\_time} = \text{base\_time} \times \text{multiplier}$$
where LOW = $1.0\times$, MEDIUM = $1.5\times$, and HIGH = $2.5\times$. When Dijkstra or A* runs under `Fastest Time` mode, edge costs are evaluated dynamically via `edge.getCost()`. This causes congested roads to become unfavorable, automatically diverting the route.

#### Q18: How does road blockage simulation demonstrate dynamic graph modification?
**Answer:** Each edge has a `bool blocked` flag. When a user blocks a road, `blocked` is toggled to `true`. During the relaxation loop of Dijkstra and A*, blocked edges are skipped in $\mathcal{O}(1)$ time. This simulates instantaneous detour calculation without needing to rebuild or reallocate graph data structures.

#### Q19: How did you implement the Alternative Route feature?
**Answer:** We temporarily penalize or block the critical intermediate segments of the optimal path and re-run the search on the modified graph. If a viable secondary path exists that differs from the primary route, the system presents both routes and explains the trade-off (e.g. "Primary route is 1.4 km shorter and 3.5 min faster than the bypass corridor").

#### Q20: How does your code handle edge cases like Source == Destination or Disconnected Nodes?
**Answer:**
- **Source == Destination:** Checks `if (src == dest)` at the entry of the algorithm and returns an immediate path of length 1 with 0.0 distance and 0.0 time in $\mathcal{O}(1)$.
- **Disconnected / Isolated Nodes:** If the priority queue empties before settling the destination, the algorithm returns `found = false` and displays a user-friendly diagnostic message.

---

### Category 6: Time & Space Complexities Summary

| Algorithm | Time Complexity | Space Complexity |
| :--- | :--- | :--- |
| **Dijkstra's Algorithm** | $\mathcal{O}((V + E) \log V)$ | $\mathcal{O}(V)$ |
| **A\* Search Algorithm** | $\mathcal{O}(E)$ best case, $\mathcal{O}((V + E) \log V)$ worst case | $\mathcal{O}(V)$ |
| **BFS Traversal** | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ |
| **DFS Traversal** | $\mathcal{O}(V + E)$ | $\mathcal{O}(V)$ |
| **Adjacency List** | Neighbors: $\mathcal{O}(\text{deg}(u))$ | $\mathcal{O}(V + E)$ |
