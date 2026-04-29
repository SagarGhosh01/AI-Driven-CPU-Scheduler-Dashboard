#pragma once

#include "Scheduler/IScheduler.h"
#include "ML/Predictor.h"
#include <memory>

class MLScheduler : public IScheduler {
private:
    std::shared_ptr<Predictor> predictor;

public:
    explicit MLScheduler(std::shared_ptr<Predictor> pred) : predictor(std::move(pred)) {}

    // Uses the Predictor to guess burst times, then schedules non-preemptively
    // based on Shortest Predicted Job First.
    std::vector<Process> schedule(const std::vector<Process>& processes) override;
};
