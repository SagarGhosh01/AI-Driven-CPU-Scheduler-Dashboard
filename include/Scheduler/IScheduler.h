// ============================================================================
// ISCHEDULER INTERFACE
// ============================================================================
// Abstract base class defining the contract for all scheduling algorithms.
// Enables polymorphic scheduling of different algorithms: FCFS, SJF, RoundRobin, AI-SJF.
//
// DESIGN PATTERN: Strategy Pattern
// - IScheduler is the abstract strategy interface
// - FCFSScheduler, SJFScheduler, RoundRobinScheduler, etc. are concrete strategies
// - Each implements different scheduling logic while maintaining same interface
// ============================================================================

#pragma once

#include <vector>
#include "ML/Process.h"

// ============================================================================
// ISCHEDULER INTERFACE
// ============================================================================
// Abstract base class for CPU scheduling algorithms.
// All schedulers inherit from this and implement the schedule() method.
//
// RESPONSIBILITY:
// Take an unscheduled process list and assign execution timeline (startTime, 
// completionTime) to each process according to scheduling algorithm logic.
// ============================================================================
class IScheduler {
public:
    // Virtual destructor: required for proper cleanup of polymorphic objects
    // Ensures derived class destructors are called when deleting base pointers
    virtual ~IScheduler() = default;

    // ========================================================================
    // SCHEDULE METHOD (Pure Virtual - Must be Implemented)
    // ========================================================================
    // Executes the scheduling algorithm to assign CPU execution times.
    //
    // INPUT: 
    //   processes - vector of processes with:
    //     - pid, arrivalTime, actualBurst, burstHistory (and predictions)
    //     - startTime, completionTime: UNINITIALIZED
    //
    // OUTPUT: 
    //   Returns a new vector of processes with:
    //     - startTime: when CPU begins executing the process
    //     - completionTime: when CPU finishes the process
    //     - All other fields copied from input
    //
    // ALGORITHM VARIES BY IMPLEMENTATION:
    //   FCFS: Execute processes in arrival order (simple, no reordering)
    //   SJF: Execute shortest job first (optimal for average turnaround time)
    //   RoundRobin: Execute in time slices (good for response time)
    //   AI-SJF: Use ML predictions to estimate and schedule shortest jobs
    //
    // KEY CONTRACT:
    // - All processes must be scheduled (no process left out)
    // - No process can execute before its arrival time
    // - No two processes can execute simultaneously
    // - Execution must be continuous (no gaps unless idle waiting for arrival)
    // 
    // EXAMPLE:
    // Input:  processes with arrivalTime, actualBurst
    // Output: same processes with startTime and completionTime filled
    // ========================================================================
    virtual std::vector<Process> schedule(const std::vector<Process>& processes) = 0;
};
