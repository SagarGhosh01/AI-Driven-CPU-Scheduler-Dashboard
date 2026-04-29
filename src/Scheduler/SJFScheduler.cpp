#include "Scheduler/SJFScheduler.h"
#include <algorithm>
#include <queue>

// ============================================================================
// SJF SCHEDULER IMPLEMENTATION
// ============================================================================
// Uses a priority queue (min-heap) to efficiently manage shortest job selection
// ============================================================================

std::vector<Process> SJFScheduler::schedule(const std::vector<Process>& processes) {
    // === STEP 1: HANDLE EMPTY PROCESS LIST ===
    // Edge case: if no processes, return empty list
    if (processes.empty()) return {};

    // === STEP 2: COPY AND SORT BY ARRIVAL TIME ===
    // Work on a copy to avoid modifying original
    // Sort by arrival so we can iterate through in order
    std::vector<Process> unscheduled = processes;
    std::sort(unscheduled.begin(), unscheduled.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    // === STEP 3: INITIALIZE DATA STRUCTURES ===
    std::vector<Process> scheduledProcesses;
    scheduledProcesses.reserve(processes.size());

    // === STEP 4: CREATE PRIORITY QUEUE (MIN-HEAP) ===
    // Min-heap ordered by burst time (smallest first)
    // Lambda comparator: returns true if a's burst > b's burst
    // (std::priority_queue is max-heap by default, so we reverse it)
    auto cmp = [](const Process& a, const Process& b) {
        // Primary ordering: by burst time (ascending)
        if (a.actualBurst == b.actualBurst) {
            // Tie-breaker: if bursts equal, earlier arrival wins
            // (FCFS tiebreaker for processes with identical burst time)
            return a.arrivalTime > b.arrivalTime;
        }
        return a.actualBurst > b.actualBurst;
    };
    std::priority_queue<Process, std::vector<Process>, decltype(cmp)> readyQueue(cmp);

    // === STEP 5: SIMULATION LOOP ===
    // Run until all processes are scheduled
    double currentTime = 0.0;
    size_t currentIndex = 0;  // Index in unscheduled list

    while (currentIndex < unscheduled.size() || !readyQueue.empty()) {
        // ====== HANDLE CPU IDLE TIME ======
        // If no processes in ready queue but processes still unscheduled,
        // fast-forward time to when next process arrives (no idle overhead)
        if (readyQueue.empty() && currentTime < unscheduled[currentIndex].arrivalTime) {
            currentTime = unscheduled[currentIndex].arrivalTime;
        }

        // ====== ADD NEWLY ARRIVED PROCESSES TO READY QUEUE ======
        // Add all processes that have arrived by currentTime
        // to the ready queue (they're now ready to execute)
        while (currentIndex < unscheduled.size() && unscheduled[currentIndex].arrivalTime <= currentTime) {
            readyQueue.push(unscheduled[currentIndex]);
            currentIndex++;
        }

        // ====== EXECUTE SHORTEST JOB ======
        if (!readyQueue.empty()) {
            // Pop process with shortest burst time from heap
            Process p = readyQueue.top();
            readyQueue.pop();

            // Assign execution timeline
            p.startTime = currentTime;
            p.completionTime = currentTime + p.actualBurst;
            
            // Update current time: move to when this process finishes
            currentTime = p.completionTime;

            // Add to scheduled list (in execution order)
            scheduledProcesses.push_back(p);
        }
    }

    // === STEP 6: RETURN SCHEDULED PROCESSES ===
    // Processes in scheduledProcesses are in execution order (not PID order)
    return scheduledProcesses;
}
