// ============================================================================
// METRICS ENGINE CLASS
// ============================================================================
// Computes performance metrics for processes after scheduling.
// Analyzes how well the scheduler performed.
//
// KEY METRICS:
// 1. Waiting Time: Time in ready queue (before CPU access)
// 2. Turnaround Time: Total time from arrival to completion
// 3. Response Time: Time until first CPU access (for interactivity)
//
// USE: After scheduler assigns startTime/completionTime,
//      MetricsEngine computes derived metrics and generates reports.
// ============================================================================

#pragma once

#include <vector>
#include "ML/Process.h"

// ============================================================================
// METRICS ENGINE CLASS
// ============================================================================
// Static utility class for computing and aggregating process metrics.
// ============================================================================
class MetricsEngine {
public:
    // ========================================================================
    // COMPUTE METRICS METHOD
    // ========================================================================
    // Calculates waiting time, turnaround time, and response time for each process.
    // These are derived metrics based on scheduler-assigned times.
    //
    // ASSUMPTIONS:
    // - startTime and completionTime are already set by scheduler
    // - arrivalTime is process metadata
    // - actualBurst is process metadata
    //
    // METRICS COMPUTED:
    //
    // 1. TURNAROUND TIME = completionTime - arrivalTime
    //    - Total time from when process arrives to completion
    //    - Lower is better (measure of responsiveness)
    //    - Average turnaround time: primary metric for scheduler comparison
    //
    // 2. WAITING TIME = turnaroundTime - actualBurst
    //    - Time spent in ready queue (NOT executing)
    //    - actualBurst is CPU time actually used
    //    - Lower is better (less idle time)
    //
    // 3. RESPONSE TIME = startTime - arrivalTime
    //    - Time until first CPU access (first I/O)
    //    - Critical for interactive systems
    //    - Lower is better (user feels responsive)
    //
    // EDGE CASE HANDLING:
    // - Floating point imprecision: small negative waiting times → set to 0
    //
    // EXAMPLE:
    // Process arrives at 5, starts at 8, completes at 18, actualBurst = 10
    // - Turnaround = 18 - 5 = 13
    // - Waiting = 13 - 10 = 3
    // - Response = 8 - 5 = 3
    // ========================================================================
    static void computeMetrics(std::vector<Process>& processes);

    // ========================================================================
    // GET AVERAGE WAITING TIME
    // ========================================================================
    // Computes average waiting time across all processes
    //
    // FORMULA: sum of all waiting times / number of processes
    //
    // SIGNIFICANCE: Lower average is better
    // - Good scheduler minimizes CPU idle time
    // - Processes spend less time waiting in queue
    // 
    // RETURNS: Average waiting time (double), 0.0 if empty
    // ========================================================================
    static double getAverageWaitingTime(const std::vector<Process>& processes);

    // ========================================================================
    // GET AVERAGE TURNAROUND TIME
    // ========================================================================
    // Computes average turnaround time across all processes
    //
    // FORMULA: sum of all turnaround times / number of processes
    //
    // SIGNIFICANCE: 
    // - PRIMARY METRIC for scheduler comparison
    // - Represents overall system efficiency
    // - Lower is better
    // - SJF is theoretically optimal (minimizes this)
    //
    // RETURNS: Average turnaround time (double), 0.0 if empty
    // ========================================================================
    static double getAverageTurnaroundTime(const std::vector<Process>& processes);

    // ========================================================================
    // GET AVERAGE RESPONSE TIME
    // ========================================================================
    // Computes average response time across all processes
    //
    // FORMULA: sum of all response times / number of processes
    //
    // SIGNIFICANCE:
    // - Important for interactive systems
    // - Measures user-perceived responsiveness
    // - Lower is better
    // - Round Robin typically has better response time than FCFS/SJF
    //
    // RETURNS: Average response time (double), 0.0 if empty
    // ========================================================================
    static double getAverageResponseTime(const std::vector<Process>& processes);
};
