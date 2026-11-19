# DAAAA — Task Scheduling Optimization System

A comprehensive C implementation of the **Job Sequencing with Deadlines** problem using **Greedy** and **Brute-Force** algorithms for optimality verification.

## 📁 Repository Contents

- **`TaskSchedulingOptimization/`** — Complete C source code, headers, data, and build system.
  - `src/`: Modular C99 implementations (`job.c`, `greedy.c`, `bruteforce.c`, `compare.c`, `visual.c`, `main.c`).
  - `include/`: Module interfaces and data structures.
  - `data/`: Benchmark datasets (`jobs.txt`).
  - `Makefile`: Automated GCC build target (`make`, `./task_scheduler`).
  - `index.html`: Interactive web dashboard and visualizer (served on `http://localhost:9090`).

---

## 👥 Classroom Challenge Team

| Member | Role | Module |
|---|---|---|
| **Sarathi R** | Formulate jobs, deadlines, profits | `job.c` (input & data) |
| **Sohali Priyanka** | Demonstrate greedy selection | `greedy.c` |
| **Selvasiva S** | Test alternative schedules, compare profits | `bruteforce.c`, `compare.c` |
| **Mukesh** | Build full system and visualization | `main.c`, `visual.c`, `Makefile` |

---

## 🚀 Quickstart

### 1. Build and Run the C Project
```bash
cd TaskSchedulingOptimization
make
./task_scheduler
```

### 2. View the Local Web Dashboard (Port 9090)
```bash
python3 -m http.server 9090
```
Open **[http://localhost:9090](http://localhost:9090)** in your browser.
