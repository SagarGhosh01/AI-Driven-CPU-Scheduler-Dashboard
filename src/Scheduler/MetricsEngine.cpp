#include "Scheduler/MetricsEngine.h"

// ============================================================================
// METRICS ENGINE IMPLEMENTATION
// ============================================================================
// Computes performance metrics for each process and aggregates for reporting
// ============================================================================

void MetricsEngine::computeMetrics(std::vector<Process>& processes) {
    // === ITERATE THROUGH EACH PROCESS ===
    // Compute metrics for process in execution order (or any order, doesn't matter)
    for (auto& p : processes) {
        // ====== CALCULATE TURNAROUND TIME ======
        // Turnaround Time = Completion Time - Arrival Time
        // Total elapsed time from when process arrives to when it finishes
        //
        // Example: Arrives at 0, completes at 20
        // Turnaround = 20 - 0 = 20
        //
        // Validity check: both values must be non-negative and set
        if (p.completionTime >= 0.0 && p.arrivalTime >= 0.0) {
            p.turnaroundTime = p.completionTime - p.arrivalTime;
            
            // ====== CALCULATE WAITING TIME ======
            // Waiting Time = Turnaround Time - Actual Burst Time
            // Time spent in ready queue (NOT executing on CPU)
            //
            // Breakdown of Turnaround Time:
            //   Total Time = Waiting Time + CPU Time (actualBurst)
            //
            // Example: 
            // Turnaround = 20, ActualBurst = 10
            // Waiting = 20 - 10 = 10 (waited 10 while others used CPU)
            p.waitingTime = p.turnaroundTime - p.actualBurst;
            
            // ====== FIX FLOATING POINT IMPRECISION ======
            // Due to floating point arithmetic, waiting time might be slightly negative
            // This happens when rounding errors accumulate
            // In reality, waiting time can't be negative, so clamp to 0
            if (p.waitingTime < 0.0) p.waitingTime = 0.0; 
        }
        
        // ====== CALCULATE RESPONSE TIME ======
        // Response Time = First Start Time - Arrival Time
        // Time until process first gets CPU access
        // Critical for interactive systems (user feels this delay)
        //
        // Example:
        // Arrives at 5, starts execution at 8
        // Response = 8 - 5 = 3 (waits 3 units before first CPU access)
        //
        // In FCFS: response time = waiting time
        // In Round Robin: processes get CPU quickly, good response time
        // In SJF: short jobs get fast response, long jobs wait long
        if (p.startTime >= 0.0 && p.arrivalTime >= 0.0) {
            p.responseTime = p.startTime - p.arrivalTime;
        }
    }
}

double MetricsEngine::getAverageWaitingTime(const std::vector<Process>& processes) {
    // === HANDLE EMPTY PROCESS LIST ===
    // Can't compute average of nothing
    if (processes.empty()) return 0.0;
    
    // === SUM ALL WAITING TIMES ===
    double sum = 0.0;
    for (const auto& p : processes) {
        sum += p.waitingTime;
    }
    
    // === COMPUTE AVERAGE ===
    // Average = Total Sum / Number of Processes
    return sum / processes.size();
}

double MetricsEngine::getAverageTurnaroundTime(const std::vector<Process>& processes) {
    // === HANDLE EMPTY PROCESS LIST ===
    if (processes.empty()) return 0.0;
    
    // === SUM ALL TURNAROUND TIMES ===
    double sum = 0.0;
    for (const auto& p : processes) {
        sum += p.turnaroundTime;
    }
    
    // === COMPUTE AVERAGE ===
    // PRIMARY METRIC: Used to compare scheduler performance
    return sum / processes.size();
}

double MetricsEngine::getAverageResponseTime(const std::vector<Process>& processes) {
    // === HANDLE EMPTY PROCESS LIST ===
    if (processes.empty()) return 0.0;
    
    // === SUM ALL RESPONSE TIMES ===
    double sum = 0.0;
    for (const auto& p : processes) {
        sum += p.responseTime;
    }
    
    // === COMPUTE AVERAGE ===
    // Shows system interactivity (how quickly users see first response)
    return sum / processes.size();
}
