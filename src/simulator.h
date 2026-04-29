// ============================================================================
// SIMULATOR HEADER
// ============================================================================
// Core simulation engine for CPU scheduling with multiple algorithms and
// real-world OS process data gathering (Windows Kernel API).
//
// ARCHITECTURE:
// 1. PROCESS CLASS: Represents a single process with burst history
// 2. SCHEDULER CLASSES: FCFS, SJF, ExponentialSJF, AISJF
// 3. SIMULATOR ENGINE: Runs simulation with I/O delays and queue management
// 4. DATA GATHERING: Queries Windows OS Kernel for real process metrics
// 5. API: Functions to run all simulators and gather real process data
//
// USE CASE: Called from server.cpp via /api/simulate endpoint
// ============================================================================

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>
#include <string>

using namespace std;

// ============================================================================
// LOGENTRY STRUCTURE
// ============================================================================
// Represents a single execution event in the simulation timeline.
// Used for generating detailed execution logs and Gantt charts.
struct LogEntry {
    // time: CPU time when process starts executing this burst
    int time;
    
    // pid: Process ID being executed
    int pid;
    
    // burst_time: How long this CPU burst takes
    int burst_time;
    
    // waiting_time: How long this process waited in queue before this burst
    int waiting_time;
};

// ============================================================================
// PROCESS CLASS (LEGACY VERSION)
// ============================================================================
// Represents a process with burst history for scheduling simulation.
// NOTE: Different from ML module's Process struct; this includes scheduling logic.
class Process {
public:
    // ====================================================================
    // PROCESS IDENTITY & EXECUTION DATA
    // ====================================================================
    // pid: Process identifier (1, 2, 3, ...)
    int pid;
    
    // bursts: Vector of CPU burst times for this process
    //         Each value is how long the process needs CPU (in time units)
    // Example: bursts = [10, 15, 8, 12] means process has 4 bursts
    vector<int> bursts;
    
    // current_burst_index: Which burst is process currently on/waiting for?
    // Range: 0 to bursts.size()-1
    // When current_burst_index >= bursts.size(), process is finished
    int current_burst_index;
    
    // arrival_time: When process entered the system (simulation time)
    int arrival_time;
    
    // ====================================================================
    // ML PREDICTION DATA
    // ====================================================================
    // exp_predicted_burst: Exponential smoothing prediction
    // Updated using formula: α * actual + (1-α) * previous_prediction
    // Used by ExponentialSJF scheduler for burst estimation
    double exp_predicted_burst; 
    
    // last_exp_burst_idx: Index when exponential prediction was last updated
    // Prevents recomputing prediction for the same burst
    int last_exp_burst_idx;
    
    // ====================================================================
    // PERFORMANCE METRICS (ACCUMULATED)
    // ====================================================================
    // waiting_times: List of waiting times for each burst
    // waiting_times[i] = how long process waited before burst i
    vector<int> waiting_times;
    
    // turnaround_times: List of turnaround times for each burst
    // turnaround_times[i] = waiting_times[i] + actual burst time for burst i
    vector<int> turnaround_times;
    
    // ====================================================================
    // CONSTRUCTORS
    // ====================================================================
    // Initialize process with PID and burst history
    // Parameters:
    //   p: Process ID
    //   b: Vector of burst times
    Process(int p, vector<int> b);
    
    // ====================================================================
    // QUERY METHODS
    // ====================================================================
    
    // GET NEXT ACTUAL BURST: Returns how long current burst should take
    // Used by ideal algorithms that know actual burst times
    int get_next_actual_burst() const;
    
    // GET CURRENT AI PREDICTION: Uses Linear Regression on last 3 bursts
    // Returns predicted burst time for current burst
    // Uses matrix math: fits line through last n bursts, predicts next
    double get_current_ai_prediction() const;
    
    // IS FINISHED: Returns true if all bursts have been executed
    // Used by scheduler to remove completed processes
    bool is_finished() const;
    
    // ====================================================================
    // EXECUTION TRACKING
    // ====================================================================
    // COMPLETE CURRENT BURST: Records metrics for finished burst
    // Updates waiting_times, turnaround_times, advances to next burst
    // Parameters:
    //   current_time: Simulation time when burst completes
    //   waiting_time: How long process waited for this burst
    // Returns: The burst time of the completed burst
    int complete_current_burst(int current_time, int waiting_time);
};

// ============================================================================
// SIMULATORRESULT STRUCTURE
// ============================================================================
// Contains results from running one scheduling algorithm
struct SimulatorResult {
    // scheduler_name: Name of algorithm (e.g., "FCFS", "Ideal SJF", "AI-Predicted SJF")
    string scheduler_name;
    
    // avg_waiting_time: Average waiting time across all bursts
    // Lower is better
    double avg_waiting_time;
    
    // avg_turnaround_time: Average turnaround time across all bursts
    // Primary metric for comparing schedulers
    double avg_turnaround_time;
    
    // logs: Detailed timeline of all execution events
    // Used for visualization (Gantt charts)
    vector<LogEntry> logs;
};

// ============================================================================
// API FUNCTIONS
// ============================================================================

// ========================================================================
// GATHER REAL PROCESS DATA (WINDOWS KERNEL API)
// ========================================================================
// Queries Windows OS Kernel for real-time process execution data.
// Samples CPU usage of running processes and extracts burst patterns.
//
// HOW IT WORKS:
// 1. Call EnumProcesses() to get all active process IDs
// 2. For each process, sample CPU time multiple times (bursts_per_process)
// 3. Calculate delta between samples = estimated CPU burst time
// 4. Return list of Process objects with realistic burst patterns
//
// WINDOWS API USED:
// - EnumProcesses: Get all active process IDs
// - OpenProcess: Open process handle with query permissions
// - GetProcessTimes: Get kernel + user CPU times (in 100-nanosecond units)
// - CloseHandle: Clean up process handles
//
// PARAMETERS:
//   num_processes: How many processes to gather (e.g., 15, 20, 50)
//   bursts_per_process: How many burst samples per process (e.g., 10)
//   
// RETURNS: vector<Process> with real kernel data
//
// PREPROCESSING:
// - Converts kernel time units (100-nanosecond) to milliseconds
// - Clamps bursts to realistic range (1-200ms)
// - Filters out processes with insufficient history
// ========================================================================
vector<Process> gather_real_process_data(int num_processes, int bursts_per_process);

// ========================================================================
// RUN ALL SIMULATORS
// ========================================================================
// Executes all scheduling algorithms on the same process data and
// compares performance.
//
// ALGORITHMS RUN:
// 1. FCFS (First Come First Served) - baseline
// 2. Ideal SJF (Shortest Job First) - theoretical optimal
// 3. Exponential SJF (α=0.5) - realistic with exponential smoothing
// 4. AI-Predicted SJF - custom Linear Regression predictor
//
// PARAMETERS:
//   processes: List of processes with burst history to schedule
//
// RETURNS: vector<SimulatorResult> - one result per algorithm
//
// USE: Compare scheduler performance on same workload
// ========================================================================
vector<SimulatorResult> run_all_simulators(const vector<Process>& processes);

#endif
