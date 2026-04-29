// ============================================================================
// FEATURE EXTRACTOR CLASS
// ============================================================================
// Extracts statistical features from process burst history for ML model.
// These features serve as input to the LinearRegressor for burst prediction.
//
// KEY CONCEPT:
// Raw burst history → Statistical Features → LinearRegressor Input
//
// FEATURES EXTRACTED:
// 1. Bias term (1.0) - for intercept in linear regression
// 2. lastBurst - the most recent CPU burst
// 3. rollingMean - average of all historical bursts
// 4. stdDeviation - standard deviation (measure of variance)
// 5. burstRange - max - min (spread of burst times)
//
// USE CASE: Called by ML schedulers to convert process history into
//           feature vectors that predict future burst times
// ============================================================================

#pragma once

#include <vector>

// ============================================================================
// FEATURE EXTRACTOR CLASS
// ============================================================================
// A static utility class for extracting statistical features from process
// burst history data. All methods are static for convenience.
// ============================================================================
class FeatureExtractor {
public:
    // ========================================================================
    // EXTRACT METHOD
    // ========================================================================
    // Computes statistical features from a process's historical burst times.
    //
    // FEATURE VECTOR STRUCTURE:
    // Index 0: 1.0 (bias term for linear regression intercept)
    // Index 1: lastBurst (most recent burst time)
    // Index 2: rollingMean (average of all bursts)
    // Index 3: stdDeviation (standard deviation measuring variability)
    // Index 4: burstRange (max - min: spread of burst times)
    //
    // Input:  history - vector of historical CPU burst times
    //                  (e.g., past 10 bursts of a process)
    //
    // Output: vector<double> of 5 features ready for ML model input
    //
    // Edge Case Handling:
    // - Empty history: returns default feature set {1.0, 0.0, 0.0, 0.0, 0.0}
    // - Single burst: mean = that burst, stdDev = 0, range = 0
    //
    // EXAMPLE:
    // history = [10, 15, 12, 14, 13]
    // mean = 12.8, stdDev ≈ 1.47, range = 5
    // returns: {1.0, 13, 12.8, 1.47, 5}
    // ========================================================================
    static std::vector<double> extract(const std::vector<double>& history);
};
