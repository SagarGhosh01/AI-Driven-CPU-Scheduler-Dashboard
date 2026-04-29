// ============================================================================
// ROUND ROBIN SCHEDULER CLASS
// ============================================================================
// Time-slice based scheduling: each process gets a fixed time quantum.
//
// ALGORITHM LOGIC:
// Processes are executed in a circular queue, each getting CPU for at most
// 'quantum' time units. If a process isn't finished, it goes to back of queue.
// This is PREEMPTIVE: processes can be interrupted mid-execution.
//
// PROS:
// - Fair (equal CPU access)
// - Good response time (all processes get CPU frequently)
// - Suitable for interactive systems
// - Time quantum tuning affects performance
//
// CONS:
// - Overhead: context switching can be expensive
// - Average turnaround time may be worse than SJF
// - Must carefully choose quantum value
//
// QUANTUM EFFECTS:
// - Too small: excessive context switching overhead
// - Too large: becomes like FCFS
// - Typical: 1-100ms depending on system
//
// PERFORMANCE: Good balance between fairness and turnaround time
// USE CASE: General-purpose operating systems (Unix, Windows, Linux)
//
// EXAMPLE (quantum=5):
// Process A: burst=12
// Process B: burst=8
// Process C: burst=4
// Schedule:
// A(0-5), B(5-10), C(10-14), A(14-19), B(19-23), A(23-27)
// ============================================================================

#pragma once

#include "Scheduler/IScheduler.h"

// ============================================================================
// ROUND ROBIN SCHEDULER CLASS
// ============================================================================
// Implements time-slice (time quantum) based scheduling.
// Each process receives a fixed time quantum before being preempted.
// ============================================================================
class RoundRobinScheduler : public IScheduler {
private:
    // ====================================================================
    // quantum: Time slice duration
    // - Each process executes for at most 'quantum' time units
    // - If process not finished, it goes to back of ready queue
    // - Typical values: 1-100ms
    // ====================================================================
    double quantum;

public:
    // ====================================================================
    // CONSTRUCTOR
    // ====================================================================
    // Initialize with specific time quantum value
    // 
    // Parameter: q - time slice duration (e.g., 10.0 for 10ms quantum)
    // 
    // EXAMPLE: RoundRobinScheduler rr(5.0);  // 5ms time quantum
    // ====================================================================
    explicit RoundRobinScheduler(double q) : quantum(q) {}
    
    // ====================================================================
    // SET QUANTUM METHOD
    // ====================================================================
    // Dynamically change time quantum if needed
    // Allows performance tuning without creating new scheduler
    // ====================================================================
    void setQuantum(double q) { quantum = q; }

    // ====================================================================
    // SCHEDULE METHOD (Round Robin Implementation)
    // ====================================================================
    // Assigns CPU execution times using circular time-slice scheduling.
    //
    // ALGORITHM:
    // 1. Maintain ready queue (regular queue, not heap)
    // 2. For each process in queue:
    //    - Execute for min(quantum, remaining burst time)
    //    - If not finished: add back to end of queue
    //    - If finished: move to completed list
    // 3. Repeat until all processes complete
    //
    // DATA STRUCTURE: Deque (double-ended queue)
    // - Push at back when preempted
    // - Pop from front when selected
    // - O(n) complexity overall
    //
    // TRACKING: RRProcess wrapper tracks:
    // - Original process data
    // - Remaining burst time (decreases each slice)
    // - First execution flag (for response time calculation)
    // ====================================================================
    std::vector<Process> schedule(const std::vector<Process>& processes) override;
};
