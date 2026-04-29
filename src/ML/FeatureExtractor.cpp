#include "ML/FeatureExtractor.h"
#include <numeric>
#include <cmath>
#include <algorithm>

// ============================================================================
// FEATURE EXTRACTION IMPLEMENTATION
// ============================================================================
// Converts historical burst data into numerical features suitable for ML
// ============================================================================

std::vector<double> FeatureExtractor::extract(const std::vector<double>& history) {
    // === STEP 1: HANDLE EMPTY HISTORY ===
    // If no history is available (new process), return default features
    // This prevents crashes and provides reasonable defaults
    // Default: bias=1.0, lastBurst=0, mean=0, stdDev=0, range=0
    if (history.empty()) {
        return {1.0, 0.0, 0.0, 0.0, 0.0};
    }

    // === STEP 2: EXTRACT LAST BURST ===
    // The most recent burst time (last element of history)
    // Often a good predictor: recent performance is indicative of future behavior
    double lastBurst = history.back();
    
    // === STEP 3: CALCULATE ROLLING MEAN (AVERAGE) ===
    // Sum all burst times and divide by count to get average
    // std::accumulate: sums all elements in the vector
    // Formula: mean = Σ(all bursts) / count
    double sum = std::accumulate(history.begin(), history.end(), 0.0);
    double rollingMean = sum / history.size();
    
    // === STEP 4: CALCULATE STANDARD DEVIATION ===
    // Measures how much burst times vary from the mean
    // High stdDev = burst times are unpredictable (high variance)
    // Low stdDev = burst times are consistent (low variance)
    //
    // Algorithm:
    // 1. Calculate squared difference from mean for each burst
    // 2. Sum all squared differences
    // 3. Divide by count to get variance
    // 4. Take square root to get standard deviation
    //
    // Formula: stdDev = sqrt(Σ((burst_i - mean)^2) / N)
    double sqSum = 0.0;
    for (double val : history) {
        double diff = val - rollingMean;
        sqSum += diff * diff;  // accumulate squared differences
    }
    double stdDeviation = std::sqrt(sqSum / history.size());
    
    // === STEP 5: CALCULATE BURST RANGE ===
    // Range = max burst time - min burst time
    // Captures the spread/variability of burst times
    // Used to understand process behavior: consistent vs. erratic
    //
    // std::minmax_element returns both min and max iterators in one pass
    // More efficient than calling min and max separately
    auto [minIt, maxIt] = std::minmax_element(history.begin(), history.end());
    double burstRange = *maxIt - *minIt;

    // === STEP 6: RETURN FEATURE VECTOR ===
    // Return all features in a standardized format:
    // [bias, lastBurst, rollingMean, stdDeviation, burstRange]
    // The LinearRegressor expects exactly this format
    return {1.0, lastBurst, rollingMean, stdDeviation, burstRange};
}
