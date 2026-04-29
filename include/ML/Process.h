// ============================================================================
// PROCESS STRUCTURE
// ============================================================================
// Represents a single process in the CPU scheduling simulation.
// This structure holds both process metadata and scheduling/execution metrics.
// ============================================================================

#pragma once

#include <vector>

// ============================================================================
// PROCESS STRUCT
// ============================================================================
// A data structure containing all information about a process:
// 1. Identity: PID and arrival time
// 2. Burst History: historical CPU burst times for prediction
// 3. Predictions: ML-predicted burst time
// 4. Execution Timeline: start/completion times set by scheduler
// 5. Performance Metrics: waiting time, turnaround time, response time
// ============================================================================
struct Process {
    // ========== PROCESS IDENTITY ==========
    // pid: Process ID (unique identifier for the process)
    int pid;
    
    // arrivalTime: When the process enters the ready queue
    // (relative time in the simulation)
    double arrivalTime;
    
    // actualBurst: The true CPU burst time for this process
    // (known after execution, used for scheduling decisions in SJF)
    double actualBurst;
    
    // burstHistory: Historical record of past burst times
    // Used by FeatureExtractor to generate features for ML model
    // Format: vector of previous CPU burst durations
    std::vector<double> burstHistory;
    
    // ========== ML PREDICTIONS ==========
    // predictedBurst: Predicted CPU burst time from LinearRegressor
    // Used by AI-Predicted SJF to make intelligent scheduling decisions
    double predictedBurst = 0.0;
    
    // ========== EXECUTION TIMELINE (Set by Scheduler) ==========
    // startTime: When the CPU starts executing this process
    // Value: -1.0 means not yet assigned
    double startTime = -1.0;
    
    // completionTime: When the CPU finishes executing this process
    // Value: -1.0 means not yet completed
    double completionTime = -1.0;
    
    // ========== PERFORMANCE METRICS (Computed by MetricsEngine) ==========
    // waitingTime: Total time process spent in ready queue
    // Formula: turnaroundTime - actualBurst
    // Lower is better (less idle time)
    double waitingTime = 0.0;
    
    // turnaroundTime: Total time from arrival to completion
    // Formula: completionTime - arrivalTime
    // Key metric for overall system performance
    double turnaroundTime = 0.0;
    
    // responseTime: Time until first CPU access
    // Formula: startTime - arrivalTime
    // Important for interactive/real-time systems
    double responseTime = 0.0;
    
    // ========== CONSTRUCTORS ==========
    // Default constructor
    Process() = default;
    
    // Parametrized constructor: Initialize process with all required data
    Process(int id, double arrival, double burst, const std::vector<double>& history)
        : pid(id), arrivalTime(arrival), actualBurst(burst), burstHistory(history) {}
};
