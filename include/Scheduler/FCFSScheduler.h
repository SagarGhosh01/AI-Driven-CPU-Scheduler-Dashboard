// ============================================================================
// FCFS SCHEDULER CLASS
// ============================================================================
// First Come First Served - the simplest CPU scheduling algorithm.
//
// ALGORITHM LOGIC:
// Processes are executed in the exact order they arrive (FIFO queue).
// No reordering, no preemption, no intelligence.
//
// PROS:
// - Extremely simple to understand and implement
// - Fair (no process is starved)
// - No overhead (no prediction needed)
//
// CONS:
// - Poor average turnaround time if short jobs follow long jobs
// - Convoy effect: short jobs wait for long jobs to complete
// - Not suitable for interactive systems (poor response time)
//
// PERFORMANCE: Baseline for comparison against other algorithms
// USE CASE: Educational (learning scheduling), batch processing systems
//
// EXAMPLE:
// Process A: arrival=0, burst=10
// Process B: arrival=1, burst=5
// FCFS Schedule:
//   A: start=0, completion=10 (waits for nothing)
//   B: start=10, completion=15 (waits 9 time units for A)
// ============================================================================

#pragma once

#include "Scheduler/IScheduler.h"

// ============================================================================
// FCFS SCHEDULER CLASS
// ============================================================================
// Implements the First Come First Served scheduling algorithm.
// Processes are scheduled exactly in order of arrival (earliest first).
// ============================================================================
class FCFSScheduler : public IScheduler {
public:
    // ====================================================================
    // SCHEDULE METHOD (Implementation of IScheduler interface)
    // ====================================================================
    // Assigns CPU execution times based on arrival order.
    //
    // ALGORITHM:
    // 1. Sort processes by arrival time (earliest first)
    // 2. For each process in order:
    //    - If CPU is idle: start immediately
    //    - If CPU is busy: start when CPU becomes free
    //    - Calculate completion time: startTime + actualBurst
    //
    // EXAMPLE WITH ARRIVALS:
    // Process P1: arrival=0, burst=5 → start=0, end=5
    // Process P2: arrival=2, burst=3 → start=5, end=8  (waits 3 units)
    // Process P3: arrival=1, burst=4 → start=8, end=12 (waits 11 units!)
    // ====================================================================
    std::vector<Process> schedule(const std::vector<Process>& processes) override;
};
