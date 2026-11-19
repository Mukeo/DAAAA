# Task Scheduling Optimization System (Job Sequencing with Deadlines)

A modular, high-performance C implementation of the **Job Sequencing with Deadlines** problem using both a **Greedy Algorithm** and an **Exhaustive Brute-Force Algorithm** for mathematical optimality verification.

---

## 👥 Project Team Roles

| Member | Role | Module |
|---|---|---|
| **Sarathi R** | Formulate jobs, deadlines, profits | `src/job.c`, `include/job.h` |
| **Sohali Priyanka** | Greedy selection and step-by-step placement | `src/greedy.c`, `include/greedy.h` |
| **Selvasiva S** | Brute-force optimal verification & comparison | `src/bruteforce.c`, `src/compare.c` |
| **Mukesh** | System architecture, visualization & build automation | `src/main.c`, `src/visual.c`, `Makefile` |

---

## 📌 Problem Statement

Given a set of $n$ jobs where each job $i$ has:
- A unique identifier and name
- A deadline $d_i \ge 1$ (1 unit of time required per job)
- A profit $p_i > 0$ earned only if completed on or before its deadline

**Objective:** Find an optimal schedule that finishes jobs within their deadlines to maximize total profit.

---

## 🧠 Algorithms & Complexity Analysis

### 1. Greedy Algorithm
- **Strategy:** Sort all jobs in descending order of profit. For each job, find the latest available time slot $\le$ deadline. If found, schedule it; otherwise, reject it.
- **Time Complexity:** $O(n \log n)$ or $O(n^2)$ with slot search.
- **Space Complexity:** $O(m)$ where $m$ is the maximum deadline.

### 2. Brute-Force Algorithm
- **Strategy:** Generates all $2^n$ subsets of jobs, validates feasibility (checking if all jobs in the subset can meet deadlines), and selects the subset with maximum profit.
- **Time Complexity:** $O(n \cdot 2^n)$
- **Purpose:** Rigorously verifies that the greedy schedule achieves the exact theoretical optimal profit.

---

## 📂 Project Structure

```
TaskSchedulingOptimization/
├── Makefile             # C build configuration (gcc -std=c99)
├── README.md            # Project documentation and specifications
├── index.html           # Web UI and visualizer
├── data/
│   └── jobs.txt         # Benchmark sample jobs dataset
├── include/
│   ├── job.h            # Job struct definitions and I/O headers
│   ├── greedy.h         # Greedy algorithm declarations
│   ├── bruteforce.h     # Brute-force verification declarations
│   ├── compare.h        # Benchmark and timing comparison headers
│   └── visual.h         # Gantt chart and ASCII bar visualizers
└── src/
    ├── job.c            # File parsing and random generation
    ├── greedy.c         # Profit-descending greedy placement
    ├── bruteforce.c     # Exhaustive subset exploration
    ├── compare.c        # Performance and optimality verification
    ├── visual.c         # Text Gantt chart & ASCII bar charts
    └── main.c           # Interactive CLI menu
```

---

## ⚙️ How to Build & Run

### 1. Build the Project
```bash
make
```

### 2. Run the Application
```bash
./task_scheduler
```

### 3. Clean Build Artifacts
```bash
make clean
```

---

## 🌐 Local Web Dashboard (Port 9090)

To launch the project web viewer and interactive simulator:
```bash
python3 -m http.server 9090
```
Then visit: **[http://localhost:9090](http://localhost:9090)**
