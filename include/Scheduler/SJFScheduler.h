// ============================================================================
// SJF SCHEDULER CLASS (SHORTEST JOB FIRST - IDEAL VERSION)
// ============================================================================
// Non-preemptive Shortest Job First: the THEORETICALLY OPTIMAL algorithm.
//
// ALGORITHM LOGIC:
// Among all processes that have arrived, always execute the one with the
// shortest burst time next. Once a process starts, it runs to completion.
// No preemption (can't interrupt mid-execution).
//
// KEY ASSUMPTION: Perfect knowledge of future burst times!
// This is impossible in real systems, but serves as a theoretical baseline.
//
// PROS:
// - Mathematically optimal: minimizes average turnaround time
// - Minimizes average waiting time
// - Simple logic
//
// CONS:
// - Impossible to implement perfectly (need to know future)
// - Unfair to long-running processes (starvation possible)
// - Doesn't help with response time
//
// PERFORMANCE: Theoretical best case for average turnaround time
// USE CASE: Baseline for comparing other algorithms
// COMPARISON: 
// - FCFS may be 20-30% worse
// - AI-SJF tries to approximate this using predictions
//
// EXAMPLE:
// Process A: arrival=0, burst=10
// Process B: arrival=1, burst=5
// Process C: arrival=2, burst=3
// SJF Schedule (optimal):
//   C: start=2, completion=5 (waits 0)
//   B: start=5, completion=10 (waits 4)
//   A: start=10, completion=20 (waits 10)
// Average waiting = (0+4+10)/3 = 4.67
// ============================================================================

#pragma once

#include "Scheduler/IScheduler.h"

// ============================================================================
// SJF SCHEDULER CLASS
// ============================================================================
// Implements non-preemptive Shortest Job First scheduling with perfect
// knowledge of actual burst times (ideal/theoretical version).
// ============================================================================
class SJFScheduler : public IScheduler {
public:
    // ====================================================================
    // SCHEDULE METHOD (Non-preemptive SJF Implementation)
    // ====================================================================
    // Assigns CPU execution times based on shortest burst time first.
    //
    // ALGORITHM:
    // 1. Maintain a ready queue (min-heap) ordered by burst time
    // 2. For each time instant:
    //    - Add all processes that have arrived by this time to ready queue
    //    - Pick process with shortest burst (top of heap)
    //    - Execute it to completion
    //    - Move to next process
    // 3. Handle gaps: if CPU idle but processes haven't arrived yet,
    //    skip to next arrival time
    //
    // DATA STRUCTURE: Priority Queue (min-heap)
    // - Top element: process with shortest burst time
    // - Efficient: O(n log n) complexity
    // ====================================================================
    std::vector<Process> schedule(const std::vector<Process>& processes) override;
};
