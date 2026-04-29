#include "ML/Predictor.h"
#include "ML/FeatureExtractor.h"
#include <stdexcept>
#include <algorithm>

void Predictor::train(const std::vector<Process>& historicalProcesses) {
    if (historicalProcesses.empty()) {
        throw std::invalid_argument("Cannot train on empty historical process list");
    }

    std::vector<std::vector<double>> X_data;
    std::vector<double> y_data;

    X_data.reserve(historicalProcesses.size());
    y_data.reserve(historicalProcesses.size());

    for (const auto& p : historicalProcesses) {
        // We only train on processes that have some history to learn from.
        if (!p.burstHistory.empty()) {
            X_data.push_back(FeatureExtractor::extract(p.burstHistory));
            y_data.push_back(p.actualBurst);
        }
    }

    if (X_data.empty()) {
        throw std::invalid_argument("No valid history found in processes to extract features");
    }

    regressor.train(X_data, y_data);
}

double Predictor::predict(const std::vector<double>& history) const {
    if (!isReady()) {
        throw std::logic_error("Predictor is not trained yet");
    }

    auto features = FeatureExtractor::extract(history);
    double prediction = regressor.predict(features);

    // Sanity clamp: a burst time cannot be zero or negative.
    // If the linear model predicts < 0 due to edge cases, clamp to a small epsilon.
    return std::max(0.1, prediction);
}
