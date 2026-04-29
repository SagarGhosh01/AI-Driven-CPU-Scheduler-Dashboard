#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>
#include <string>

using namespace std;

struct LogEntry {
    int time;
    int pid;
    int burst_time;
    int waiting_time;
};

class Process {
public:
    int pid;
    vector<int> bursts;
    int current_burst_index;
    int arrival_time;
    
    double exp_predicted_burst; 
    int last_exp_burst_idx;
    
    vector<int> waiting_times;
    vector<int> turnaround_times;
    
    Process(int p, vector<int> b);
    
    int get_next_actual_burst() const;
    double get_current_ai_prediction() const;
    bool is_finished() const;
    int complete_current_burst(int current_time, int waiting_time);
};

struct SimulatorResult {
    string scheduler_name;
    double avg_waiting_time;
    double avg_turnaround_time;
    vector<LogEntry> logs;
};

// Data gathering via Windows API
vector<Process> gather_real_process_data(int num_processes, int bursts_per_process);

// Run simulation
vector<SimulatorResult> run_all_simulators(const vector<Process>& processes);

#endif
