#include "Scheduler/RoundRobinScheduler.h"
#include <algorithm>
#include <deque>

// ============================================================================
// ROUND ROBIN SCHEDULER IMPLEMENTATION
// ============================================================================
// Uses a double-ended queue (deque) to efficiently manage circular scheduling
// ============================================================================

// ============================================================================
// HELPER STRUCTURE: RRProcess
// ============================================================================
// Wrapper to track Round Robin specific information during execution
struct RRProcess {
    // Original process data
    Process p;
    
    // How much burst time remains to be executed
    // Decreases by quantum amount each time process runs
    double remainingBurst;
    
    // Flag: true if this is first execution (for response time calculation)
    // Response time = when first CPU access happens - arrival time
    bool isFirstTime;
};

std::vector<Process> RoundRobinScheduler::schedule(const std::vector<Process>& processes) {
    // === STEP 1: HANDLE EMPTY PROCESS LIST ===
    if (processes.empty()) return {};

    // === STEP 2: COPY AND SORT BY ARRIVAL TIME ===
    std::vector<Process> unscheduled = processes;
    std::sort(unscheduled.begin(), unscheduled.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    // === STEP 3: INITIALIZE DATA STRUCTURES ===
    std::vector<Process> scheduledProcesses;
    scheduledProcesses.reserve(processes.size());

    // Ready queue: deque allows efficient push_back and pop_front
    // (implementing circular queue behavior)
    std::deque<RRProcess> readyQueue;
    
    double currentTime = 0.0;
    size_t currentIndex = 0;

    // === STEP 4: SIMULATION LOOP ===
    while (currentIndex < unscheduled.size() || !readyQueue.empty()) {
        
        // ====== HANDLE CPU IDLE TIME ======
        // If ready queue is empty but processes still unscheduled,
        // fast-forward time to when next process arrives
        if (readyQueue.empty() && currentTime < unscheduled[currentIndex].arrivalTime) {
            currentTime = unscheduled[currentIndex].arrivalTime;
        }

        // ====== ADD NEWLY ARRIVED PROCESSES TO READY QUEUE ======
        // Add all processes that have arrived by currentTime
        // Wrap them in RRProcess with remainingBurst = actualBurst
        while (currentIndex < unscheduled.size() && unscheduled[currentIndex].arrivalTime <= currentTime) {
            readyQueue.push_back({
                unscheduled[currentIndex],           // Original process
                unscheduled[currentIndex].actualBurst, // Full burst time remaining
                true                                  // First execution
            });
            currentIndex++;
        }

        // ====== EXECUTE NEXT PROCESS IN QUEUE ======
        if (!readyQueue.empty()) {
            // Get front process (front of the queue)
            RRProcess currentRR = readyQueue.front();
            readyQueue.pop_front();

            // ====== RECORD START TIME (FIRST EXECUTION ONLY) ======
            // Record when process first gets CPU access
            // Used later to calculate response time
            if (currentRR.isFirstTime) {
                currentRR.p.startTime = currentTime;
                currentRR.isFirstTime = false;
            }

            // ====== CALCULATE EXECUTION TIME THIS SLICE ======
            // Execute for minimum of:
            // 1. quantum: time slice allocation (e.g., 10ms)
            // 2. remainingBurst: what process actually needs
            // Example: quantum=10, remainingBurst=5 → execute 5
            double executeTime = std::min(quantum, currentRR.remainingBurst);
            
            // ====== UPDATE TIME AND REMAINING BURST ======
            // Advance global time
            currentTime += executeTime;
            
            // Decrease remaining work for this process
            currentRR.remainingBurst -= executeTime;

            // ====== CRITICAL: CHECK FOR NEW ARRIVALS DURING EXECUTION ======
            // Important: while this process was executing, new processes may have arrived
            // Add them to ready queue BEFORE deciding whether to preempt current process
            // This ensures fairness: newly arrived processes don't jump ahead of already-ready processes
            while (currentIndex < unscheduled.size() && unscheduled[currentIndex].arrivalTime <= currentTime) {
                readyQueue.push_back({
                    unscheduled[currentIndex],
                    unscheduled[currentIndex].actualBurst,
                    true
                });
                currentIndex++;
            }

            // ====== CHECK IF PROCESS IS FINISHED ======
            if (currentRR.remainingBurst > 1e-9) {
                // Still work remaining: use floating point safe comparison (> 0)
                // Add process back to end of queue (preempted)
                readyQueue.push_back(currentRR);
            } else {
                // Process finished: record completion time and move to completed list
                currentRR.p.completionTime = currentTime;
                scheduledProcesses.push_back(currentRR.p);
            }
        }
    }

    // === STEP 5: SORT BY COMPLETION TIME ===
    // Round Robin may complete processes out of PID order
    // Sort by completion time to show natural finish order
    // (Though other sorting strategies are possible)
    std::sort(scheduledProcesses.begin(), scheduledProcesses.end(), [](const Process& a, const Process& b) {
        return a.completionTime < b.completionTime;
    });

    // === STEP 6: RETURN SCHEDULED PROCESSES ===
    return scheduledProcesses;
}
