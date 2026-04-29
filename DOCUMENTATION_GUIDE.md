# Code Documentation Guide
## AI-Driven CPU Scheduler Dashboard

This document lists all files with comprehensive inline comments explaining logic, algorithms, and design decisions.

---

## 📚 FULLY DOCUMENTED FILES

### 1. **ML Module (Machine Learning & Data)**

#### `include/ML/Process.h` ✅
- **Purpose**: Core process data structure for scheduling
- **Key Sections**:
  - Process identity (PID, arrival time)
  - Burst history for ML predictions
  - ML prediction data (predictedBurst)
  - Execution timeline (startTime, completionTime)
  - Performance metrics (waitingTime, turnaroundTime, responseTime)
- **Explains**: What each field means and why it's needed

#### `include/ML/LinearRegressor.h` ✅
#### `src/ML/LinearRegressor.cpp` ✅
- **Purpose**: Custom native C++ machine learning model
- **Algorithm**: Linear Regression using SVD (Singular Value Decomposition)
- **Key Concepts**:
  - Normal Equation for least-squares fitting: w = (X^T * X)^(-1) * X^T * y
  - Training process with numerical stability
  - Prediction: dot product of weights and features (w^T * x)
  - Why SVD is better than direct inversion
- **Explains**: 
  - 6-step training process with matrix operations
  - Feature extraction and vectorization
  - Prediction computation with fallback handling

#### `include/ML/FeatureExtractor.h` ✅
#### `src/ML/FeatureExtractor.cpp` ✅
- **Purpose**: Convert process burst history into ML features
- **Features Extracted**:
  1. Bias term (1.0) - for intercept
  2. Last burst - most recent execution time
  3. Rolling mean - average of all bursts
  4. Standard deviation - variability measure
  5. Burst range - max - min spread
- **Explains**:
  - Why each feature is important
  - Statistical formulas (mean, variance, standard deviation)
  - Edge case handling (empty history)
  - Real example of feature computation

#### `include/ML/ProcessGenerator.h` ✅
#### `src/ML/ProcessGenerator.cpp` ✅
- **Purpose**: Generate realistic synthetic processes for simulation
- **Workload Types**:
  - CPU_BOUND: 20-100ms bursts (compute-heavy)
  - IO_BOUND: 1-15ms bursts (I/O-heavy)
  - MIXED: 1-100ms bursts (realistic general)
- **Explains**:
  - Process generation pipeline
  - Random number generation
  - Arrival time distribution
  - Why sorting by arrival time is critical
  - How each workload type differs

---

### 2. **Scheduler Module (CPU Scheduling Algorithms)**

#### `include/Scheduler/IScheduler.h` ✅
- **Purpose**: Abstract interface for all schedulers
- **Design Pattern**: Strategy Pattern
- **Key Contract**:
  - Input: unscheduled processes with arrival times
  - Output: processes with startTime and completionTime assigned
  - No process overlaps, no execution before arrival
- **Explains**: Polymorphic scheduling design

#### `include/Scheduler/FCFSScheduler.h` ✅
#### `src/Scheduler/FCFSScheduler.cpp` ✅
- **Algorithm**: First Come First Served (FIFO)
- **Pros**: Simple, fair, no overhead
- **Cons**: Poor turnaround time, convoy effect
- **Use Case**: Educational baseline, batch processing
- **Explains**:
  - Why FCFS is a bad algorithm for interactive systems
  - How to handle arrival times and idle CPU
  - Complete execution timeline algorithm

#### `include/Scheduler/SJFScheduler.h` ✅
#### `src/Scheduler/SJFScheduler.cpp` ✅
- **Algorithm**: Shortest Job First (Ideal/Theoretical)
- **Assumption**: Perfect knowledge of future burst times
- **Data Structure**: Priority Queue (min-heap by burst time)
- **Pros**: Optimal for average turnaround time
- **Cons**: Impossible in real systems, unfair to long jobs
- **Explains**:
  - Why SJF is theoretically optimal
  - Priority queue implementation
  - Tie-breaking with arrival time
  - Gap handling when CPU idle

#### `include/Scheduler/RoundRobinScheduler.h` ✅
#### `src/Scheduler/RoundRobinScheduler.cpp` ✅
- **Algorithm**: Time-slice based preemptive scheduling
- **Key Feature**: Quantum (time slice) tuning
- **Data Structure**: Double-ended queue (deque)
- **Pros**: Fair, good response time
- **Cons**: Context switch overhead, worse turnaround than SJF
- **Helper Structure**: RRProcess wrapper to track remaining burst
- **Explains**:
  - Quantum effects (too small = overhead, too large = FCFS)
  - Preemption and context switching
  - Arrival handling during execution
  - How to track first execution for response time

#### `include/Scheduler/MetricsEngine.h` ✅
#### `src/Scheduler/MetricsEngine.cpp` ✅
- **Purpose**: Calculate performance metrics for processes
- **Metrics Computed**:
  1. **Turnaround Time** = completion - arrival (total elapsed)
  2. **Waiting Time** = turnaround - burst (queue time)
  3. **Response Time** = start - arrival (first access)
- **Significance**:
  - Turnaround: primary metric for scheduler comparison
  - Waiting: measures efficiency
  - Response: important for interactive systems
- **Explains**:
  - Formulas and derivations
  - Why each metric matters
  - Average calculation
  - Floating point imprecision handling

---

### 3. **Simulation Engine (Core Logic)**

#### `src/simulator.h` ✅
- **Purpose**: Main simulation engine header
- **Classes**:
  - Process: represents a single process
  - Scheduler: abstract scheduler interface
  - FCFS, SJF, ExponentialSJF, AISJF: concrete implementations
  - SimulatorEngine: orchestrates simulation
- **Structures**:
  - LogEntry: execution event in timeline
  - SimulatorResult: algorithm performance results
- **Key Methods**:
  - get_next_actual_burst(): returns actual CPU time
  - get_current_ai_prediction(): uses Linear Regression ML
  - is_finished(): checks process completion
  - complete_current_burst(): records metrics
- **Explains**:
  - Process lifecycle in simulation
  - Scheduling algorithm interface
  - Metrics collection

#### `src/simulator.cpp` ✅
- **Purpose**: Simulation implementation and data gathering
- **Key Algorithms**:
  - **get_current_ai_prediction()**: Custom Linear Regression
    - Fits line through last 3 bursts
    - Predicts next burst using y = mx + c
  - **gather_real_process_data()**: Windows Kernel API
    - Uses EnumProcesses to get all PIDs
    - Samples process times via GetProcessTimes
    - Extracts burst patterns from kernel data
- **Explains**:
  - Matrix math for regression (Σx, Σy, Σxy, Σx²)
  - Slope and intercept calculation
  - Windows API process telemetry
  - Time unit conversions (100-nanosecond to milliseconds)

---

### 4. **HTTP Server & API**

#### `src/server.cpp` ✅
- **Purpose**: Native C++ HTTP server using Winsock2
- **Architecture**:
  - Windows Sockets initialization
  - TCP socket binding to port 5000
  - Client connection acceptance loop
  - Request parsing and routing
- **Endpoints**:
  - `GET /`: serves index.html
  - `GET /templates/*`, `GET /static/*`: serves frontend files
  - `POST /api/simulate`: runs scheduling simulation
- **Request Handler Logic**:
  - Parse HTTP method and path
  - Extract JSON request body
  - Validate parameters
  - Run simulation
  - Build JSON response
  - Error handling with 404/405/500 responses
- **Explains**:
  - HTTP protocol and headers
  - Winsock2 socket programming
  - Request/response lifecycle
  - JSON serialization/deserialization
  - Error handling patterns

---

## 🎯 KEY ALGORITHMS EXPLAINED

### Linear Regression (ML Prediction)
**File**: `src/simulator.cpp` - `get_current_ai_prediction()`
- **Formula**: y = mx + c
- **Coefficients**:
  - m (slope) = (n·Σxy - Σx·Σy) / (n·Σx² - (Σx)²)
  - c (intercept) = (Σy - m·Σx) / n
- **Stability**: Handles division by zero (no variance)
- **Prediction**: y_pred = m·n + c

### SJF with Priority Queue
**File**: `src/Scheduler/SJFScheduler.cpp`
- **Data Structure**: Min-heap priority queue
- **Ordering**: By actualBurst time (ascending)
- **Complexity**: O(n log n)
- **Tie-breaker**: Earlier arrival time wins

### Round Robin with Preemption
**File**: `src/Scheduler/RoundRobinScheduler.cpp`
- **Data Structure**: Double-ended queue (deque)
- **Quantum Effect**: min(quantum, remainingBurst)
- **Key Step**: Check arrivals DURING execution
- **Fairness**: Circular queue ensures equal access

### Metrics Calculation
**File**: `src/Scheduler/MetricsEngine.cpp`
- **Turnaround**: completionTime - arrivalTime
- **Waiting**: turnaroundTime - actualBurst
- **Response**: startTime - arrivalTime
- **Aggregation**: Sum / count for averages

---

## 📊 DATA FLOW

```
1. Data Gathering (server.cpp)
   ├─ HTTP POST request with num_processes
   └─ gather_real_process_data() uses Windows Kernel API

2. Process Generation
   ├─ Each process gets burst history
   ├─ Features extracted via FeatureExtractor
   └─ Sorted by arrival time

3. ML Model
   ├─ LinearRegressor trained on burst history
   └─ Makes predictions for future bursts

4. Scheduling
   ├─ FCFS: simple arrival order
   ├─ SJF: ideal shortest burst
   ├─ RoundRobin: time-slice based
   └─ AI-SJF: uses ML predictions

5. Metrics Calculation
   ├─ MetricsEngine computes times
   ├─ Collects logs for visualization
   └─ Aggregates statistics

6. Response (server.cpp)
   └─ JSON with metrics and timeline
```

---

## 🔬 COMPARISON OF ALGORITHMS

| Algorithm | Turnaround | Fairness | Response | Realistic |
|-----------|-----------|----------|----------|-----------|
| FCFS | Poor | Good | Poor | Yes |
| Ideal SJF | **Best** | Poor | Poor | No |
| Round Robin | Moderate | **Best** | **Best** | Yes |
| AI-SJF | Good | Moderate | Moderate | Yes |

---

## 💡 KEY LEARNING POINTS

1. **Scheduling Trade-offs**: No single "best" algorithm; different metrics matter for different systems
2. **ML Prediction**: Useful but imperfect; exponential smoothing and linear regression both have limits
3. **Performance Metrics**: Wait time, turnaround, and response time measure different aspects
4. **Data Structures**: Priority queues (heap) for SJF, deques for circular buffers
5. **Systems Programming**: Windows Kernel API, Sockets, HTTP protocols
6. **Native Development**: C++ without heavy frameworks; direct OS integration

---

## 🎓 EXPLANATION CHECKLIST FOR SIR

✅ All files have detailed comments  
✅ Every function has purpose, parameters, return values documented  
✅ All algorithms explained with formulas and step-by-step logic  
✅ Data structures justified with complexity analysis  
✅ Edge cases and error handling documented  
✅ Examples provided where appropriate  
✅ Trade-offs and design decisions explained  

**Ready to present!**
