# Nexus OS: AI-Driven CPU Scheduler Dashboard
**Project Architecture & Technical Report**

---

## 1. Executive Summary
The Nexus OS CPU Scheduler is a highly optimized, native application designed to simulate and visualize operating system CPU scheduling heuristics. Originally prototyped in Python, the system has been entirely re-architected into a **high-performance standalone C++ environment**. 

It natively queries the Windows OS Kernel for real-time process telemetry, predicts future CPU burst times using a custom-built Linear Regression Machine Learning model, and serves a premium, interactive web dashboard entirely from a raw C++ Winsock HTTP server.

---

## 2. System Architecture

The project fundamentally operates on a Client-Server model, but with zero external dependencies (no Python, no Node.js, no external web servers).

### 2.1 Backend (Native C++)
The entire backend compiles down to a single executable (`server.exe`).
- **`src/server.cpp`**: Implements a single-threaded raw HTTP server using the Windows Sockets API (`winsock2.h`). It routes web traffic, serves static HTML/CSS/JS files directly from the disk, and exposes the `/api/simulate` REST endpoint.
- **`src/simulator.cpp`**: Contains the core logic for the OS simulation. It implements multiple scheduling algorithms and manages the process queues.
- **Data Serialization**: Uses the lightweight, header-only `nlohmann/json` (`json.hpp`) library to parse incoming API requests and serialize simulation results into JSON for the frontend.

### 2.2 Frontend (HTML5 / Vanilla JS / CSS3)
The frontend is a completely decoupled Single Page Application (SPA) that communicates with the C++ backend via asynchronous `fetch()` API calls.
- **UI/UX Design**: Built with a dark "FinTech" aesthetic utilizing deep glassmorphism (`backdrop-filter`), CSS Grid layouts, and smooth micro-animations.
- **Visualizations**: 
  - **ApexCharts**: Used to render the highly complex, interactive Gantt Charts that display the exact timeline of CPU execution.
  - **Chart.js**: Used to render the comparative Bar Chart (Wait Time vs Turnaround Time) and the Doughnut Chart for Process Workload Distribution.

---

## 3. Core Features & Innovations

### 3.1 Live Kernel Telemetry (`<psapi.h>`)
Instead of generating random, synthetic data for the simulation, the C++ engine connects directly to the Windows OS Kernel using `EnumProcesses` and `GetProcessTimes`. It actively samples the real-time execution speeds of background applications running on your physical machine, creating a highly realistic dataset for the scheduler to manage.

### 3.2 Native Machine Learning Engine
To predict how long a process will need the CPU (Burst Time prediction), the system does not rely on external heavy libraries like TensorFlow or Scikit-Learn. Instead, `Process::get_current_ai_prediction()` implements a **custom Linear Regression algorithm** written purely in C++. It analyzes the historical variance of the last 3 bursts of a process using matrix math to predict the next burst with high speed and low overhead.

### 3.3 Scheduling Algorithms Implemented
The engine pits several heuristic algorithms against each other using the exact same real-world data:
1. **FCFS (First Come First Serve)**: The baseline standard. Processes are executed exactly in the order they arrive.
2. **Ideal SJF (Shortest Job First)**: A theoretical algorithm that perfectly knows the exact future burst times. Used as the absolute optimal baseline.
3. **Exponential SJF**: A realistic predictive algorithm that guesses the next burst using Exponential Smoothing ($\alpha = 0.5$) based on previous bursts.
4. **AI-Predicted SJF**: The flagship algorithm that utilizes the Native C++ Linear Regression engine to predict bursts and intelligently reorder the ready queue.

---

## 4. Performance & Execution

Because the backend is written in compiled C++, the overhead of running simulations on hundreds of processes is virtually zero. 

### How to Run
1. Open the project root directory.
2. Ensure you are on a Windows machine (required for `<windows.h>` and kernel telemetry).
3. Execute `server.exe`.
4. The C++ server will bind to port 5000. Open a web browser and navigate to `http://127.0.0.1:5000`.
5. Use the slider on the left to configure how many real OS processes to sample, and click **Execute Simulation**.

### Automated Reporting
The dashboard features a **Download Report** function. Upon execution, the Javascript engine can instantly compile the simulation timeline into a highly detailed, readable ASCII table format (`.txt`), allowing engineers to easily review the exact chronological sequence of CPU execution and the performance summary of each algorithm.

---

## 5. Conclusion
This project successfully demonstrates the fusion of low-level Systems Programming (C++, Sockets, Kernel APIs) with modern Web Development and Machine Learning. By stripping away heavy frameworks (like Python/Flask) and writing the core logic natively, the CPU Scheduler achieves peak performance, zero dependencies, and an incredibly responsive user experience.
