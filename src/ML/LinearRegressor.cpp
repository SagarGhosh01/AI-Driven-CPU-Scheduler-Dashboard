#include "ML/LinearRegressor.h"
#include <stdexcept>

// ============================================================================
// TRAIN METHOD: Fits the Linear Regression model using the Normal Equation
// ============================================================================
// Purpose: Learn the optimal weights (coefficients) from training data using
//          the least-squares method with SVD decomposition for numerical stability
// Input:
//   - X_data: feature matrix (each row is a sample, each column is a feature)
//   - y_data: target values vector (expected outputs for each sample)
// ============================================================================
void LinearRegressor::train(const std::vector<std::vector<double>>& X_data, const std::vector<double>& y_data) {
    // === STEP 1: VALIDATE INPUT DATA ===
    // Check that training data is not empty and dimensions match
    // X_data and y_data must have the same number of samples (rows)
    if (X_data.empty() || y_data.empty() || X_data.size() != y_data.size()) {
        throw std::invalid_argument("Invalid training data: dimensions mismatch or empty");
    }

    // === STEP 2: EXTRACT DIMENSIONS ===
    // Get the number of training samples and features from input data
    // numSamples: total number of training examples
    // numFeatures: number of independent variables (features) per sample
    size_t numSamples = X_data.size();
    size_t numFeatures = X_data[0].size();

    // === STEP 3: CONVERT STANDARD C++ VECTORS TO EIGEN MATRICES ===
    // Eigen is a high-performance linear algebra library optimized for matrix operations
    // X: Feature matrix of size (numSamples x numFeatures)
    // y: Target vector of size (numSamples x 1)
    Eigen::MatrixXd X(numSamples, numFeatures);
    Eigen::VectorXd y(numSamples);

    // === STEP 4: POPULATE EIGEN MATRICES FROM INPUT VECTORS ===
    // Iterate through each training sample and copy data to Eigen structures
    for (size_t i = 0; i < numSamples; ++i) {
        // Validate that each feature vector has consistent dimensionality
        // All samples must have the same number of features
        if (X_data[i].size() != numFeatures) {
            throw std::invalid_argument("Inconsistent feature vector size in training data");
        }
        // Copy feature values for sample i
        for (size_t j = 0; j < numFeatures; ++j) {
            X(i, j) = X_data[i][j];
        }
        // Copy target value for sample i
        y(i) = y_data[i];
    }

    // === STEP 5: SOLVE THE NORMAL EQUATION USING SVD ===
    // Normal Equation: w = (X^T * X)^(-1) * X^T * y
    // This formula minimizes the sum of squared errors (least-squares optimization)
    // 
    // WHY SVD (Singular Value Decomposition)?
    // - Numerically stable and robust even for ill-conditioned matrices
    // - Handles rank-deficient cases gracefully (avoids singular matrix errors)
    // - bdcSvd = Bidiagonal Divide and Conquer SVD (fast and accurate)
    // - solve(y) performs least-squares fitting using the SVD decomposition
    //
    // The result: 'weights' vector contains the learned coefficients for each feature
    weights = X.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(y);

    // === STEP 6: MARK MODEL AS TRAINED ===
    // Set flag to true indicating model is ready for making predictions
    // Predict method checks this flag to prevent usage before training
    isTrained = true;
}

// ============================================================================
// PREDICT METHOD: Makes predictions on new data using learned weights
// ============================================================================
// Purpose: Apply the trained linear regression model to new feature vectors
// Input:  features - a vector of feature values for a single sample
// Output: Predicted target value (scalar double)
// Logic: Compute dot product of learned weights with input features: w^T * x
// ============================================================================
double LinearRegressor::predict(const std::vector<double>& features) const {
    // === STEP 1: VALIDATE MODEL STATE ===
    // Ensure model has been trained before attempting predictions
    // Prevents undefined behavior from using uninitialized weights
    if (!isTrained) {
        throw std::logic_error("Predict called before model was trained");
    }
    
    // === STEP 2: VALIDATE INPUT DIMENSIONS ===
    // Feature vector size must match the number of features the model was trained on
    // Prevents dimension mismatch in dot product calculation
    if (features.size() != static_cast<size_t>(weights.size())) {
        throw std::invalid_argument("Feature size mismatch during prediction");
    }

    // === STEP 3: CONVERT INPUT FEATURES TO EIGEN VECTOR ===
    // Convert from std::vector<double> to Eigen::VectorXd for efficient computation
    Eigen::VectorXd x_vec(features.size());
    for (size_t i = 0; i < features.size(); ++i) {
        x_vec(i) = features[i];
    }

    // === STEP 4: COMPUTE PREDICTION ===
    // Linear regression prediction: y_pred = w^T * x
    // dot(x_vec) performs the dot product between weights and input features
    // Result: scalar value representing the predicted target
    return weights.dot(x_vec);
}
