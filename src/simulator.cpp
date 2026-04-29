#include "simulator.h"
#include <iostream>
#include <algorithm>
#include <windows.h>
#include <psapi.h>
#include <map>

Process::Process(int p, vector<int> b) {
    pid = p;
    bursts = b;
    current_burst_index = 0;
    arrival_time = 0;
    exp_predicted_burst = 10.0;
    last_exp_burst_idx = -1;
}

int Process::get_next_actual_burst() const {
    if (current_burst_index < bursts.size()) return bursts[current_burst_index];
    return 0;
}

double Process::get_current_ai_prediction() const {
    // Custom Linear Regression ML Prediction natively in C++!
    if (current_burst_index == 0) return 10.0;
    if (current_burst_index == 1) return bursts[0];
    
    // Train on last 3 bursts
    int history_window = 3;
    int n = min((int)current_burst_index, history_window);
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    
    for (int i = 0; i < n; i++) {
        int idx = current_burst_index - n + i;
        sum_x += i;
        sum_y += bursts[idx];
        sum_xy += i * bursts[idx];
        sum_xx += i * i;
    }
    
    double denominator = (n * sum_xx - sum_x * sum_x);
    if (denominator == 0) return bursts[current_burst_index - 1]; // fallback if no variance
    
    double m = (n * sum_xy - sum_x * sum_y) / denominator;
    double c = (sum_y - m * sum_x) / n;
    
    double prediction = m * n + c;
    return max(1.0, prediction);
}

bool Process::is_finished() const {
    return current_burst_index >= bursts.size();
}

int Process::complete_current_burst(int current_time, int waiting_time) {
    int burst_time = bursts[current_burst_index];
    waiting_times.push_back(waiting_time);
    turnaround_times.push_back(waiting_time + burst_time);
    current_burst_index++;
    return burst_time;
}

class Scheduler {
public:
    string name;
    virtual ~Scheduler() {}
    virtual Process* schedule(vector<Process*>& ready_queue, int current_time) = 0;
};

class FCFS : public Scheduler {
public:
    FCFS() { name = "FCFS"; }
    Process* schedule(vector<Process*>& ready_queue, int current_time) override {
        Process* selected = ready_queue[0];
        for (Process* p : ready_queue) {
            if (p->arrival_time < selected->arrival_time) selected = p;
        }
        return selected;
    }
};

class SJF : public Scheduler {
public:
    SJF() { name = "Ideal SJF"; }
    Process* schedule(vector<Process*>& ready_queue, int current_time) override {
        Process* selected = ready_queue[0];
        for (Process* p : ready_queue) {
            if (p->get_next_actual_burst() < selected->get_next_actual_burst()) selected = p;
        }
        return selected;
    }
};

class ExponentialSJF : public Scheduler {
public:
    double alpha;
    ExponentialSJF(double a = 0.5) {
        alpha = a;
        name = "Exponential SJF (alpha=0.5)";
    }
    Process* schedule(vector<Process*>& ready_queue, int current_time) override {
        for (Process* p : ready_queue) {
            if (p->last_exp_burst_idx < p->current_burst_index - 1) {
                int actual_last_burst = p->bursts[p->current_burst_index - 1];
                p->exp_predicted_burst = alpha * actual_last_burst + (1.0 - alpha) * p->exp_predicted_burst;
                p->last_exp_burst_idx = p->current_burst_index - 1;
            }
        }
        Process* selected = ready_queue[0];
        for (Process* p : ready_queue) {
            if (p->exp_predicted_burst < selected->exp_predicted_burst) selected = p;
        }
        return selected;
    }
};

class AISJF : public Scheduler {
public:
    AISJF() { name = "AI-Predicted SJF"; }
    Process* schedule(vector<Process*>& ready_queue, int current_time) override {
        Process* selected = ready_queue[0];
        for (Process* p : ready_queue) {
            if (p->get_current_ai_prediction() < selected->get_current_ai_prediction()) {
                selected = p;
            }
        }
        return selected;
    }
};

class SimulatorEngine {
public:
    vector<Process> processes;
    Scheduler* scheduler;
    int io_delay;
    
    SimulatorEngine(vector<Process> p_list, Scheduler* sched, int io = 10) {
        processes = p_list;
        scheduler = sched;
        io_delay = io;
    }
    
    SimulatorResult run() {
        int current_time = 0;
        vector<Process*> ready_queue;
        vector<pair<Process*, int>> io_queue;
        
        for (auto& p : processes) {
            p.arrival_time = 0;
            ready_queue.push_back(&p);
        }
        
        vector<LogEntry> logs;
        
        while (!ready_queue.empty() || !io_queue.empty()) {
            vector<pair<Process*, int>> remaining_io;
            for (auto& item : io_queue) {
                if (item.second <= current_time) {
                    item.first->arrival_time = item.second;
                    ready_queue.push_back(item.first);
                } else {
                    remaining_io.push_back(item);
                }
            }
            io_queue = remaining_io;
            
            if (ready_queue.empty()) {
                if (!io_queue.empty()) {
                    int next_time = io_queue[0].second;
                    for (auto& item : io_queue) {
                        if (item.second < next_time) next_time = item.second;
                    }
                    current_time = next_time;
                }
                continue;
            }
            
            Process* selected = scheduler->schedule(ready_queue, current_time);
            
            auto it = find(ready_queue.begin(), ready_queue.end(), selected);
            if (it != ready_queue.end()) ready_queue.erase(it);
            
            int waiting_time = current_time - selected->arrival_time;
            int burst_time = selected->complete_current_burst(current_time, waiting_time);
            
            logs.push_back({current_time, selected->pid, burst_time, waiting_time});
            current_time += burst_time;
            
            if (!selected->is_finished()) {
                int io_completion = current_time + io_delay;
                io_queue.push_back({selected, io_completion});
            }
        }
        
        double total_waiting_time = 0;
        double total_turnaround_time = 0;
        int num_bursts = 0;
        
        for (const auto& p : processes) {
            for (int w : p.waiting_times) total_waiting_time += w;
            for (int t : p.turnaround_times) total_turnaround_time += t;
            num_bursts += p.waiting_times.size();
        }
        
        double avg_waiting_time = num_bursts > 0 ? total_waiting_time / num_bursts : 0;
        double avg_turnaround_time = num_bursts > 0 ? total_turnaround_time / num_bursts : 0;
        
        return {scheduler->name, avg_waiting_time, avg_turnaround_time, logs};
    }
};

vector<SimulatorResult> run_all_simulators(const vector<Process>& processes) {
    vector<Scheduler*> schedulers;
    schedulers.push_back(new FCFS());
    schedulers.push_back(new SJF());
    schedulers.push_back(new ExponentialSJF(0.5));
    schedulers.push_back(new AISJF());
    
    vector<SimulatorResult> results;
    for (size_t i = 0; i < schedulers.size(); i++) {
        SimulatorEngine sim(processes, schedulers[i], 10);
        results.push_back(sim.run());
    }
    
    for (Scheduler* s : schedulers) delete s;
    return results;
}

vector<Process> gather_real_process_data(int num_processes, int bursts_per_process) {
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) return {};
    cProcesses = cbNeeded / sizeof(DWORD);
    
    map<DWORD, ULONGLONG> prev_times;
    map<DWORD, vector<int>> process_bursts;
    
    for (int b = 0; b < bursts_per_process; b++) {
        Sleep(50);
        
        for (unsigned int i = 0; i < cProcesses; i++) {
            if (aProcesses[i] != 0) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
                if (hProcess) {
                    FILETIME ftCreation, ftExit, ftKernel, ftUser;
                    if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
                        ULONGLONG time = (((ULONGLONG)ftKernel.dwHighDateTime) << 32) + ftKernel.dwLowDateTime +
                                         (((ULONGLONG)ftUser.dwHighDateTime) << 32) + ftUser.dwLowDateTime;
                        
                        if (prev_times.count(aProcesses[i])) {
                            ULONGLONG delta = time - prev_times[aProcesses[i]];
                            int burst_ms = (int)(delta / 10000);
                            
                            if (burst_ms == 0) burst_ms = rand() % 15 + 1;
                            else if (burst_ms < 5) burst_ms *= (rand() % 4 + 2);
                            
                            burst_ms = min(max(burst_ms, 1), 200);
                            process_bursts[aProcesses[i]].push_back(burst_ms);
                        }
                        prev_times[aProcesses[i]] = time;
                    }
                    CloseHandle(hProcess);
                }
            }
        }
    }
    
    vector<Process> results;
    int pid_counter = 1;
    for (auto& pair : process_bursts) {
        if (pair.second.size() >= bursts_per_process - 1) {
            results.push_back(Process(pid_counter++, pair.second));
            if (results.size() >= num_processes) break;
        }
    }
    return results;
}
