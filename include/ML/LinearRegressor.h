#pragma once

#include <vector>
#include <Eigen/Dense>

// ============================================================================
// LINEAR REGRESSOR CLASS
// ============================================================================
// A machine learning model that uses Linear Regression to predict CPU burst
// times for scheduling decisions.
//
// APPROACH: Least Squares Linear Regression with SVD decomposition
// - Uses Normal Equation solved via Singular Value Decomposition (SVD)
// - Numerically stable and robust against ill-conditioned matrices
// - Learns optimal weights to minimize prediction error
//
// USE CASE: Predicts how long a process will need the CPU (burst time)
//           in the AI-Predicted SJF scheduling algorithm
// ============================================================================
class LinearRegressor {
private:
    // weights: The learned model coefficients (one per feature)
    // Each weight represents how much a feature contributes to burst time prediction
    Eigen::VectorXd weights;
    
    // isTrained: Flag indicating whether the model has been trained
    // Prevents invalid predictions from being made on untrained models
    bool isTrained = false;

public:
    // Default constructor: creates an untrained regressor
    LinearRegressor() = default;

    // ========================================================================
    // TRAIN METHOD
    // ========================================================================
    // Trains the model using the Least Squares Normal Equation
    // Formula: w = (X^T * X)^(-1) * X^T * y
    //
    // Parameters:
    //   X_data: Vector of feature vectors (each row is a training sample)
    //           Each feature vector represents process characteristics
    //   y_data: The actual observed burst times (targets to fit)
    //
    // Returns: void
    // Throws: std::invalid_argument if data dimensions are invalid
    // ========================================================================
    void train(const std::vector<std::vector<double>>& X_data, const std::vector<double>& y_data);

    // ========================================================================
    // PREDICT METHOD
    // ========================================================================
    // Predicts the burst time for a given feature vector using learned weights
    // Formula: y_pred = w^T * x (dot product of weights and features)
    //
    // Parameters:
    //   features: Vector of features for a single process sample
    //
    // Returns: Predicted burst time (scalar double value)
    // Throws: std::logic_error if model not trained yet
    //         std::invalid_argument if feature size doesn't match training data
    // ========================================================================
    double predict(const std::vector<double>& features) const;

    // Getter: Returns whether the model has been trained
    bool getIsTrained() const { return isTrained; }
    
    // Getter: Returns the learned weights (useful for debugging/testing)
    // Can be used to inspect model coefficients and their relative importance
    Eigen::VectorXd getWeights() const { return weights; }
};
