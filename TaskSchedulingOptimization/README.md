# Task Scheduling Optimization System (Job Sequencing with Deadlines)

A modular C99 implementation of the **Job Sequencing with Deadlines** problem using both a **Greedy Algorithm** and an **Exhaustive Brute-Force Algorithm** for mathematical optimality verification — with a timetable-board Web UI served by the C binary itself.

---

## Project Team Roles

| Member | Role | Module |
|---|---|---|
| **Sarathi R** | Formulate jobs, deadlines, profits | `src/job.c`, `include/job.h` |
| **Sohali Priyanka** | Greedy selection and step-by-step placement | `src/greedy.c`, `include/greedy.h` |
| **Selvasiva S** | Brute-force optimal verification & comparison | `src/bruteforce.c`, `src/compare.c` |
| **Mukesh** | System architecture, web server, visualization & build automation | `src/main.c`, `src/server.c`, `src/visual.c`, `Makefile` |

---

## Problem Statement

Given a set of $n$ jobs where each job $i$ has:
- A unique identifier and name
- A deadline $d_i \ge 1$ (1 unit of time required per job)
- A profit $p_i > 0$ earned only if completed on or before its deadline

**Objective:** Find an optimal schedule that finishes jobs within their deadlines to maximize total profit.

---

## Algorithms & Complexity Analysis

### 1. Greedy Algorithm
- **Strategy:** Sort all jobs in descending order of profit. For each job, find the latest available time slot ≤ deadline. If found, schedule it; otherwise, reject it.
- **Time Complexity:** O(n²) with slot search.
- **Space Complexity:** O(m) where m is the maximum deadline.

### 2. Brute-Force Algorithm
- **Strategy:** Generates all 2ⁿ subsets of jobs, validates feasibility, and selects the subset with maximum profit.
- **Time Complexity:** O(n · 2ⁿ)
- **Purpose:** Rigorously verifies that the greedy schedule achieves the exact theoretical optimal profit.

---

## Project Structure

```
TaskSchedulingOptimization/
├── Makefile             # Build configuration (gcc -std=c99)
├── README.md            # Project documentation
├── data/
│   └── jobs.txt         # Benchmark sample jobs dataset
├── include/
│   ├── job.h            # Job struct definitions
│   ├── greedy.h         # Greedy algorithm declarations
│   ├── bruteforce.h     # Brute-force declarations
│   ├── compare.h        # Comparison headers
│   ├── visual.h         # Gantt chart visualizers
│   └── server.h         # HTTP server declarations
├── src/
│   ├── job.c            # File parsing and random generation
│   ├── greedy.c         # Profit-descending greedy placement
│   ├── bruteforce.c     # Exhaustive subset exploration
│   ├── compare.c        # Performance and optimality verification
│   ├── visual.c         # Text Gantt chart & ASCII bar charts
│   ├── main.c           # Interactive CLI menu + --serve flag
│   └── server.c         # Pure C HTTP server (POSIX sockets)
└── web/
    ├── index.html       # Timetable-board UI (single page)
    ├── styles.css       # Design system (paper, ink, vermilion)
    └── app.js           # Vanilla ES modules, no libraries
```

---

## How to Build & Run

### Build
```bash
make
```

### Run the console menu
```bash
./task_scheduler
```

### Web UI — start the C HTTP server on port 9090
```bash
make serve
```
Then open **http://localhost:9090** in your browser.

The Web UI is a timetable-board interface with five sections:
- **01 Jobs** — editable job table, load sample or generate random jobs
- **02 Greedy** — step-by-step greedy player with animation and running profit counter
- **03 Timetable** — final greedy schedule as a timetable grid
- **04 Verification** — side-by-side greedy vs brute force with VERIFIED OPTIMAL stamp
- **05 Performance** — inline SVG chart of execution time vs n (log scale)

### JSON API endpoints (all on port 9090)
| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/sample` | Returns jobs from data/jobs.txt |
| POST | `/api/jobs/random?n=8` | Generates n random jobs |
| POST | `/api/greedy` | Greedy schedule + step-by-step log |
| POST | `/api/bruteforce` | Optimal schedule + timing |
| POST | `/api/compare` | Both results + optimal verdict |
| GET | `/api/performance` | Timings for n = 5, 8, 10, 12, 15, 18 |

### Clean build artifacts
```bash
make clean
```
