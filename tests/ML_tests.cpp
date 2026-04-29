#include <gtest/gtest.h>
#include "ML/Process.h"
#include "ML/ProcessGenerator.h"
#include "ML/FeatureExtractor.h"
#include "ML/LinearRegressor.h"

TEST(ProcessGeneratorTest, GenerateCorrectNumberOfProcesses) {
    auto processes = ProcessGenerator::generate(5, 10, ProcessGenerator::WorkloadType::MIXED);
    ASSERT_EQ(processes.size(), 5);
}

TEST(ProcessGeneratorTest, CheckHistorySize) {
    auto processes = ProcessGenerator::generate(1, 10, ProcessGenerator::WorkloadType::CPU_BOUND);
    ASSERT_EQ(processes.front().burstHistory.size(), 10);
}

TEST(ProcessGeneratorTest, IsSortedByArrivalTime) {
    auto processes = ProcessGenerator::generate(10, 5, ProcessGenerator::WorkloadType::IO_BOUND);
    for (size_t i = 1; i < processes.size(); ++i) {
        EXPECT_GE(processes[i].arrivalTime, processes[i-1].arrivalTime);
    }
}

TEST(ProcessGeneratorTest, InvalidArgumentsThrow) {
    EXPECT_THROW(ProcessGenerator::generate(0, 5, ProcessGenerator::WorkloadType::MIXED), std::invalid_argument);
    EXPECT_THROW(ProcessGenerator::generate(5, -1, ProcessGenerator::WorkloadType::MIXED), std::invalid_argument);
}

TEST(FeatureExtractorTest, ExtractsCorrectly) {
    std::vector<double> history = {2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0};
    auto features = FeatureExtractor::extract(history);

    ASSERT_EQ(features.size(), 5);
    EXPECT_DOUBLE_EQ(features[0], 1.0); // Bias
    EXPECT_DOUBLE_EQ(features[1], 9.0); // Last burst
    EXPECT_DOUBLE_EQ(features[2], 5.0); // Mean (40/8)
    // Range is 9 - 2 = 7
    EXPECT_DOUBLE_EQ(features[4], 7.0); 
}

TEST(LinearRegressorTest, TrainsAndPredictsSimpleLinearRelation) {
    LinearRegressor regressor;
    
    // Simulate a perfect linear relationship: y = 2*x1 + 3*x2 + 5 (bias)
    // features = [1.0 (bias), x1, x2]
    std::vector<std::vector<double>> X = {
        {1.0, 1.0, 1.0}, // y = 5 + 2 + 3 = 10
        {1.0, 2.0, 0.0}, // y = 5 + 4 + 0 = 9
        {1.0, 0.0, 2.0}, // y = 5 + 0 + 6 = 11
        {1.0, 2.0, 2.0}  // y = 5 + 4 + 6 = 15
    };
    std::vector<double> y = {10.0, 9.0, 11.0, 15.0};

    regressor.train(X, y);

    EXPECT_TRUE(regressor.getIsTrained());

    auto weights = regressor.getWeights();
    ASSERT_EQ(weights.size(), 3);
    
    // Check if the weights converged near [5, 2, 3]
    EXPECT_NEAR(weights(0), 5.0, 1e-5);
    EXPECT_NEAR(weights(1), 2.0, 1e-5);
    EXPECT_NEAR(weights(2), 3.0, 1e-5);

    // Predict a new point: x1=3, x2=1 => y = 5 + 6 + 3 = 14
    double prediction = regressor.predict({1.0, 3.0, 1.0});
    EXPECT_NEAR(prediction, 14.0, 1e-5);
}
