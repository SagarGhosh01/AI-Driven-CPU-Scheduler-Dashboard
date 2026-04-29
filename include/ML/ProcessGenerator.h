// ============================================================================
// PROCESS GENERATOR CLASS
// ============================================================================
// Generates synthetic process data for CPU scheduling simulations.
// Creates realistic workload patterns with different burst time distributions.
//
// PURPOSE: Rather than gathering real system data every time, this generates
//          realistic synthetic processes with configurable characteristics.
//
// KEY FEATURE: Different workload types affect burst time distribution:
// - CPU_BOUND: Long CPU bursts (20-100ms) - like computation-heavy tasks
// - IO_BOUND: Short CPU bursts (1-15ms) - like I/O-heavy tasks
// - MIXED: Variable bursts (1-100ms) - realistic general workload
// ============================================================================

#pragma once

#include <vector>
#include "ML/Process.h"

// ============================================================================
// PROCESS GENERATOR CLASS
// ============================================================================
// A static utility class for generating synthetic process data with
// realistic characteristics for scheduling simulations.
// ============================================================================
class ProcessGenerator {
public:
    // ========================================================================
    // WORKLOAD TYPE ENUM
    // ========================================================================
    // Specifies the type of workload to generate different burst patterns:
    //
    // CPU_BOUND:
    //   - Represents CPU-intensive processes (e.g., calculations, graphics)
    //   - Burst times: 20-100ms (long CPU usage)
    //   - Few I/O operations
    //
    // IO_BOUND:
    //   - Represents I/O-intensive processes (e.g., disk/network operations)
    //   - Burst times: 1-15ms (short CPU usage between I/O waits)
    //   - Frequent I/O operations
    //
    // MIXED:
    //   - Represents general-purpose workload with variable characteristics
    //   - Burst times: 1-100ms (unpredictable mix)
    //   - Realistic for modern multitasking systems
    // ========================================================================
    enum class WorkloadType {
        CPU_BOUND,
        IO_BOUND,
        MIXED
    };

    // ========================================================================
    // GENERATE METHOD
    // ========================================================================
    // Generates a synthetic list of processes with realistic burst time
    // distributions based on the specified workload type.
    //
    // PARAMETERS:
    //   numProcesses: Total number of processes to generate
    //                 (e.g., 10, 20, 50 processes)
    //
    //   historySize: Number of historical bursts per process
    //                Used for ML feature extraction
    //                (e.g., 5 means each process has 5 past bursts)
    //
    //   workload: Type of workload affecting burst time distribution
    //             (CPU_BOUND, IO_BOUND, or MIXED)
    //
    // RETURN: std::vector<Process>
    //   - Processes sorted by arrival time (ascending)
    //   - Each process has: pid, arrivalTime, actualBurst, burstHistory
    //   - Ready for immediate use in scheduling simulation
    //
    // ALGORITHM:
    // 1. For each process: generate random arrival time (0-50ms)
    // 2. For each process: generate burst history (historySize bursts)
    // 3. For each process: generate actual burst time
    // 4. Sort all processes by arrival time
    // 5. Return sorted process list
    //
    // EXAMPLE:
    // auto processes = ProcessGenerator::generate(20, 5, WorkloadType::MIXED);
    // // Result: 20 processes, each with 5 historical bursts, mixed workload
    // ========================================================================
    static std::vector<Process> generate(int numProcesses, int historySize, WorkloadType workload);
};
