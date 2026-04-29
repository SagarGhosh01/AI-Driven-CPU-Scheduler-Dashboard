#include "Scheduler/FCFSScheduler.h"
#include <algorithm>

// ============================================================================
// FCFS SCHEDULER IMPLEMENTATION
// ============================================================================
// Simple implementation: just execute in arrival order with no reordering
// ============================================================================

std::vector<Process> FCFSScheduler::schedule(const std::vector<Process>& processes) {
    // === STEP 1: COPY PROCESSES ===
    // Work on a copy to avoid modifying the original
    std::vector<Process> scheduledProcesses = processes;
    
    // === STEP 2: SORT BY ARRIVAL TIME ===
    // Ensure processes are ordered by earliest arrival first
    // Although input should already be sorted, sorting here ensures correctness
    // Lambda: return true if process a arrives before process b
    std::sort(scheduledProcesses.begin(), scheduledProcesses.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    // === STEP 3: ASSIGN EXECUTION TIMES ===
    // Variable to track when the CPU becomes free
    // Starts at 0 (CPU available at time 0)
    double currentTime = 0.0;

    // === STEP 4: ITERATE THROUGH EACH PROCESS IN ARRIVAL ORDER ===
    // FCFS: no reordering, just process each in sequence
    for (auto& p : scheduledProcesses) {
        // ====== CHECK: CAN WE START NOW OR MUST WE WAIT? ======
        // If process hasn't arrived yet (currentTime < arrivalTime),
        // the CPU must idle until process arrives
        if (currentTime < p.arrivalTime) {
            // CPU was idle, process just arrived
            // Set currentTime to process arrival time
            currentTime = p.arrivalTime;
        }
        
        // ====== ASSIGN EXECUTION TIMELINE ======
        // Start time: when CPU becomes available (as determined above)
        p.startTime = currentTime;
        
        // Completion time: start time + how long the process needs
        p.completionTime = currentTime + p.actualBurst;
        
        // ====== UPDATE CPU AVAILABILITY ======
        // After this process finishes, CPU is free for next process
        currentTime = p.completionTime;
    }

    // === STEP 5: RETURN SCHEDULED PROCESSES ===
    // All processes now have startTime and completionTime assigned
    return scheduledProcesses;
}
