#include "Scheduler/MLScheduler.h"
#include <algorithm>
#include <queue>
#include <stdexcept>

std::vector<Process> MLScheduler::schedule(const std::vector<Process>& processes) {
    if (!predictor || !predictor->isReady()) {
        throw std::logic_error("MLScheduler requires a trained Predictor");
    }
    if (processes.empty()) return {};

    std::vector<Process> unscheduled = processes;
    
    // 1. Populate predictedBurst for all processes
    for (auto& p : unscheduled) {
        if (!p.burstHistory.empty()) {
            p.predictedBurst = predictor->predict(p.burstHistory);
        } else {
            // Fallback for completely new processes without history
            p.predictedBurst = p.actualBurst; // Or some global average
        }
    }

    // 2. Sort by arrival time for timeline extraction
    std::sort(unscheduled.begin(), unscheduled.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    std::vector<Process> scheduledProcesses;
    scheduledProcesses.reserve(processes.size());

    // 3. Min-heap ordered by PREDICTED burst
    auto cmp = [](const Process& a, const Process& b) {
        if (a.predictedBurst == b.predictedBurst) {
            return a.arrivalTime > b.arrivalTime;
        }
        return a.predictedBurst > b.predictedBurst;
    };
    std::priority_queue<Process, std::vector<Process>, decltype(cmp)> readyQueue(cmp);

    double currentTime = 0.0;
    size_t currentIndex = 0;

    while (currentIndex < unscheduled.size() || !readyQueue.empty()) {
        if (readyQueue.empty() && currentTime < unscheduled[currentIndex].arrivalTime) {
            currentTime = unscheduled[currentIndex].arrivalTime;
        }

        while (currentIndex < unscheduled.size() && unscheduled[currentIndex].arrivalTime <= currentTime) {
            readyQueue.push(unscheduled[currentIndex]);
            currentIndex++;
        }

        if (!readyQueue.empty()) {
            Process p = readyQueue.top();
            readyQueue.pop();

            p.startTime = currentTime;
            p.completionTime = currentTime + p.actualBurst; // ACTUAL execution still takes actual burst time!
            currentTime = p.completionTime;

            scheduledProcesses.push_back(p);
        }
    }

    return scheduledProcesses;
}
