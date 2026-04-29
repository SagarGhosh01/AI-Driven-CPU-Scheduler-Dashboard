#include "ML/ProcessGenerator.h"
#include <algorithm>
#include <stdexcept>
#include <random>

// ============================================================================
// PROCESS GENERATION IMPLEMENTATION
// ============================================================================
// Generates synthetic process data with realistic workload characteristics
// ============================================================================

std::vector<Process> ProcessGenerator::generate(int numProcesses, int historySize, WorkloadType workload) {
    // === STEP 1: VALIDATE INPUT PARAMETERS ===
    // Ensure valid parameters before attempting generation
    // numProcesses > 0: can't generate 0 or negative processes
    // historySize >= 0: history can be 0 (new process) but not negative
    if (numProcesses <= 0 || historySize < 0) {
        throw std::invalid_argument("Invalid parameters for process generation");
    }

    // === STEP 2: INITIALIZE PROCESS VECTOR ===
    // Pre-allocate space for efficiency (avoid repeated reallocations)
    std::vector<Process> processes;
    processes.reserve(numProcesses);

    // === STEP 3: SET UP RANDOM NUMBER GENERATOR ===
    // Modern C++ random utilities for high-quality random numbers
    // std::random_device: seed from OS entropy source
    // std::mt19937: Mersenne Twister engine (fast, good distribution)
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // === STEP 4: DEFINE ARRIVAL TIME DISTRIBUTION ===
    // All workload types have same arrival times: uniformly distributed 0-50ms
    // This simulates processes arriving randomly over a 50ms window
    std::uniform_real_distribution<> arrivalDist(0.0, 50.0);
    
    // === STEP 5: DEFINE BURST TIME DISTRIBUTIONS FOR EACH WORKLOAD ===
    // Different distributions based on workload type
    //
    // CPU_BOUND: 20-100ms (long bursts - compute-intensive)
    //   Example: image processing, numerical calculations
    //
    // IO_BOUND: 1-15ms (short bursts - I/O-heavy)
    //   Example: web requests, file reads, database queries
    //
    // MIXED: 1-100ms (unpredictable - realistic general workload)
    //   Example: modern applications with mixed I/O and CPU usage
    std::uniform_real_distribution<> cpuBoundDist(20.0, 100.0);
    std::uniform_real_distribution<> ioBoundDist(1.0, 15.0);
    std::uniform_real_distribution<> mixedDist(1.0, 100.0);

    // === STEP 6: GENERATE EACH PROCESS ===
    // For each process i from 0 to numProcesses-1:
    for (int i = 0; i < numProcesses; ++i) {
        // ====== 6a: GENERATE ARRIVAL TIME ======
        // Random arrival time when process enters ready queue
        double arrivalTime = arrivalDist(gen);
        
        // ====== 6b: GENERATE BURST HISTORY ======
        // Create historical burst data for ML feature extraction
        // Pre-allocate space for efficiency
        std::vector<double> history;
        history.reserve(historySize);
        
        // For each historical burst slot (typically 3-5 bursts):
        for (int h = 0; h < historySize; ++h) {
            double burst = 0.0;
            // Generate burst according to selected workload type
            switch(workload) {
                case WorkloadType::CPU_BOUND: 
                    burst = cpuBoundDist(gen);  // 20-100ms
                    break;
                case WorkloadType::IO_BOUND: 
                    burst = ioBoundDist(gen);   // 1-15ms
                    break;
                case WorkloadType::MIXED: 
                    burst = mixedDist(gen);     // 1-100ms
                    break;
            }
            history.push_back(burst);
        }
        
        // ====== 6c: GENERATE ACTUAL (CURRENT) BURST TIME ======
        // The actual burst time for this process
        // Generated from same distribution as history for consistency
        double actualBurst = 0.0;
        switch(workload) {
            case WorkloadType::CPU_BOUND: 
                actualBurst = cpuBoundDist(gen);
                break;
            case WorkloadType::IO_BOUND: 
                actualBurst = ioBoundDist(gen);
                break;
            case WorkloadType::MIXED: 
                actualBurst = mixedDist(gen);
                break;
        }

        // ====== 6d: CREATE PROCESS ====== 
        // emplace_back: construct in-place (more efficient than push_back)
        // Arguments: pid (i+1), arrivalTime, actualBurst, burstHistory
        processes.emplace_back(i + 1, arrivalTime, actualBurst, history);
    }
    
    // === STEP 7: SORT PROCESSES BY ARRIVAL TIME ===
    // CRITICAL: Schedulers expect processes sorted by arrival time
    // This simulates the natural queue order processes would have arrived in
    // Formula: ascending order (earliest arrivals first)
    // Lambda function: (a, b) returns true if a should come before b
    std::sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    // === STEP 8: RETURN GENERATED PROCESSES ===
    // Ready for use in scheduling simulation
    return processes;
}
