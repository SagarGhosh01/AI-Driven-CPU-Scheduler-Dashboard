#include <gtest/gtest.h>
#include "ML/Process.h"
#include "Scheduler/MetricsEngine.h"
#include "Scheduler/FCFSScheduler.h"
#include "Scheduler/SJFScheduler.h"
#include "Scheduler/RoundRobinScheduler.h"
#include "Scheduler/MLScheduler.h"
#include "ML/Predictor.h"

TEST(MetricsEngineTest, ComputesMetricsCorrectly) {
    std::vector<Process> processes;
    
    // Simulate a process: Arrival=0, Burst=5, Start=2, Completion=7
    Process p1;
    p1.pid = 1;
    p1.arrivalTime = 0.0;
    p1.actualBurst = 5.0;
    p1.startTime = 2.0;
    p1.completionTime = 7.0;
    
    processes.push_back(p1);

    // Simulate another process: Arrival=2, Burst=3, Start=7, Completion=10
    Process p2;
    p2.pid = 2;
    p2.arrivalTime = 2.0;
    p2.actualBurst = 3.0;
    p2.startTime = 7.0;
    p2.completionTime = 10.0;
    
    processes.push_back(p2);

    MetricsEngine::computeMetrics(processes);

    // p1 metrics
    EXPECT_DOUBLE_EQ(processes[0].turnaroundTime, 7.0); // 7 - 0
    EXPECT_DOUBLE_EQ(processes[0].waitingTime, 2.0);    // 7 - 5
    EXPECT_DOUBLE_EQ(processes[0].responseTime, 2.0);   // 2 - 0

    // p2 metrics
    EXPECT_DOUBLE_EQ(processes[1].turnaroundTime, 8.0); // 10 - 2
    EXPECT_DOUBLE_EQ(processes[1].waitingTime, 5.0);    // 8 - 3
    EXPECT_DOUBLE_EQ(processes[1].responseTime, 5.0);   // 7 - 2
}

TEST(MetricsEngineTest, ComputesAveragesCorrectly) {
    std::vector<Process> processes;
    Process p1, p2;
    p1.waitingTime = 10.0;
    p1.turnaroundTime = 20.0;
    p1.responseTime = 5.0;

    p2.waitingTime = 4.0;
    p2.turnaroundTime = 10.0;
    p2.responseTime = 1.0;
    
    processes.push_back(p1);
    processes.push_back(p2);

    EXPECT_DOUBLE_EQ(MetricsEngine::getAverageWaitingTime(processes), 7.0);
    EXPECT_DOUBLE_EQ(MetricsEngine::getAverageTurnaroundTime(processes), 15.0);
    EXPECT_DOUBLE_EQ(MetricsEngine::getAverageResponseTime(processes), 3.0);
}

TEST(ClassicalSchedulersTest, FCFSSchedulesCorrectly) {
    std::vector<Process> processes;
    Process p1; p1.pid = 1; p1.arrivalTime = 0.0; p1.actualBurst = 5.0; processes.push_back(p1);
    Process p2; p2.pid = 2; p2.arrivalTime = 1.0; p2.actualBurst = 3.0; processes.push_back(p2);

    FCFSScheduler fcfs;
    auto scheduled = fcfs.schedule(processes);

    ASSERT_EQ(scheduled.size(), 2);
    EXPECT_DOUBLE_EQ(scheduled[0].startTime, 0.0);
    EXPECT_DOUBLE_EQ(scheduled[0].completionTime, 5.0);

    EXPECT_DOUBLE_EQ(scheduled[1].startTime, 5.0);
    EXPECT_DOUBLE_EQ(scheduled[1].completionTime, 8.0);
}

TEST(ClassicalSchedulersTest, SJFSchedulesCorrectly) {
    std::vector<Process> processes;
    Process p1; p1.pid = 1; p1.arrivalTime = 0.0; p1.actualBurst = 10.0; processes.push_back(p1);
    Process p2; p2.pid = 2; p2.arrivalTime = 1.0; p2.actualBurst = 2.0; processes.push_back(p2);
    Process p3; p3.pid = 3; p3.arrivalTime = 2.0; p3.actualBurst = 1.0; processes.push_back(p3);

    SJFScheduler sjf;
    auto scheduled = sjf.schedule(processes);

    ASSERT_EQ(scheduled.size(), 3);
    
    // p1 arrives first, runs 0 to 10.
    EXPECT_EQ(scheduled[0].pid, 1);
    EXPECT_DOUBLE_EQ(scheduled[0].startTime, 0.0);
    EXPECT_DOUBLE_EQ(scheduled[0].completionTime, 10.0);
    
    // At t=10, p2(burst=2) and p3(burst=1) are ready. p3 runs next.
    EXPECT_EQ(scheduled[1].pid, 3);
    EXPECT_DOUBLE_EQ(scheduled[1].startTime, 10.0);
    EXPECT_DOUBLE_EQ(scheduled[1].completionTime, 11.0);

    // Then p2 runs.
    EXPECT_EQ(scheduled[2].pid, 2);
    EXPECT_DOUBLE_EQ(scheduled[2].startTime, 11.0);
    EXPECT_DOUBLE_EQ(scheduled[2].completionTime, 13.0);
}

TEST(ClassicalSchedulersTest, RoundRobinSchedulesCorrectly) {
    std::vector<Process> processes;
    Process p1; p1.pid = 1; p1.arrivalTime = 0.0; p1.actualBurst = 5.0; processes.push_back(p1);
    Process p2; p2.pid = 2; p2.arrivalTime = 1.0; p2.actualBurst = 4.0; processes.push_back(p2);

    RoundRobinScheduler rr(2.0); // Quantum = 2
    auto scheduled = rr.schedule(processes);

    ASSERT_EQ(scheduled.size(), 2);
    // RR Sequence:
    // t=0: p1 runs for 2 (rem=3). Arr: p2(at 1). Queue: p2.
    // t=2: p1 pushed back. Queue: p2, p1
    // t=2: p2 runs for 2 (rem=2).
    // t=4: p2 pushed back. Queue: p1, p2
    // t=4: p1 runs for 2 (rem=1).
    // t=6: p1 pushed back. Queue: p2, p1
    // t=6: p2 runs for 2 (rem=0). p2 finishes at 8.
    // t=8: p1 runs for 1 (rem=0). p1 finishes at 9.

    auto findProcess = [&](int pid) {
        return std::find_if(scheduled.begin(), scheduled.end(), [pid](const Process& p) { return p.pid == pid; });
    };

    auto res1 = findProcess(1);
    auto res2 = findProcess(2);

    ASSERT_NE(res1, scheduled.end());
    ASSERT_NE(res2, scheduled.end());

    EXPECT_DOUBLE_EQ(res1->startTime, 0.0);
    EXPECT_DOUBLE_EQ(res1->completionTime, 9.0);

    EXPECT_DOUBLE_EQ(res2->startTime, 2.0);
    EXPECT_DOUBLE_EQ(res2->completionTime, 8.0);
}

TEST(MLSchedulerTest, SchedulesBasedOnPredictions) {
    // 1. Setup a Predictor
    auto predictor = std::make_shared<Predictor>();
    
    // Train it with dummy data: assume predictedBurst roughly equals actualBurst for simplicity
    std::vector<Process> trainingData;
    Process t1; t1.burstHistory = {5.0}; t1.actualBurst = 5.0; trainingData.push_back(t1);
    Process t2; t2.burstHistory = {2.0}; t2.actualBurst = 2.0; trainingData.push_back(t2);
    predictor->train(trainingData);

    EXPECT_TRUE(predictor->isReady());

    // 2. Setup processes to schedule
    std::vector<Process> processes;
    
    // p1 arrives at 0, actual burst is 10, but history suggests it's 10
    Process p1; p1.pid = 1; p1.arrivalTime = 0.0; p1.actualBurst = 10.0; p1.burstHistory = {10.0}; 
    processes.push_back(p1);
    
    // p2 arrives at 1, actual burst is 8, but history suggests 2! (ML predicts it's shorter)
    Process p2; p2.pid = 2; p2.arrivalTime = 1.0; p2.actualBurst = 8.0; p2.burstHistory = {2.0};
    processes.push_back(p2);
    
    // p3 arrives at 2, actual burst is 1, history suggests 1
    Process p3; p3.pid = 3; p3.arrivalTime = 2.0; p3.actualBurst = 1.0; p3.burstHistory = {1.0};
    processes.push_back(p3);

    // 3. Schedule
    MLScheduler ml(predictor);
    auto scheduled = ml.schedule(processes);

    ASSERT_EQ(scheduled.size(), 3);
    
    // Timeline:
    // t=0: p1 arrives and starts. Runs until t=10.
    EXPECT_EQ(scheduled[0].pid, 1);
    EXPECT_DOUBLE_EQ(scheduled[0].startTime, 0.0);
    EXPECT_DOUBLE_EQ(scheduled[0].completionTime, 10.0);
    
    // At t=10, both p2 and p3 are ready.
    // p2's PREDICTED burst is ~2.0. p3's PREDICTED burst is ~1.0.
    // So p3 should go next! Even though p2's ACTUAL burst is 8.0.
    // Wait, let's verify if prediction makes p3 < p2. Yes.
    EXPECT_EQ(scheduled[1].pid, 3);
    EXPECT_DOUBLE_EQ(scheduled[1].startTime, 10.0);
    EXPECT_DOUBLE_EQ(scheduled[1].completionTime, 11.0); // actual burst is 1
    
    // Then p2 runs (actual burst is 8, so finishes at 19)
    EXPECT_EQ(scheduled[2].pid, 2);
    EXPECT_DOUBLE_EQ(scheduled[2].startTime, 11.0);
    EXPECT_DOUBLE_EQ(scheduled[2].completionTime, 19.0);
}
