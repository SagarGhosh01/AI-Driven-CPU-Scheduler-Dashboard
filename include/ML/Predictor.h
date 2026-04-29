#pragma once

#include <vector>
#include "ML/Process.h"
#include "ML/LinearRegressor.h"

class Predictor {
private:
    LinearRegressor regressor;

public:
    Predictor() = default;

    // Train the regressor using historical processes.
    // Extracts features from each process's burstHistory and uses actualBurst as target.
    void train(const std::vector<Process>& historicalProcesses);

    // Predict the burst time for a given history.
    double predict(const std::vector<double>& history) const;
    
    // Check if the predictor is ready to be used
    bool isReady() const { return regressor.getIsTrained(); }
};
