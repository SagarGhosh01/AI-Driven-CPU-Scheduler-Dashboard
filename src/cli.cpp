// ============================================================================
// SIMULATOR CLI ENTRY POINT
// ============================================================================
// Takes JSON simulation requests via stdin, runs the C++ simulation,
// and outputs the results as JSON to stdout.
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "json.hpp"
#include "simulator.h"

using namespace std;
using json = nlohmann::json;

int main() {
    try {
        // Read JSON input from stdin
        string input_str;
        string line;
        while (getline(cin, line)) {
            input_str += line;
        }

        int num_processes = 20;
        int bursts_per_process = 10;
        
        if (!input_str.empty()) {
            auto j_req = json::parse(input_str);
            if (j_req.contains("num_processes")) {
                num_processes = j_req["num_processes"];
            }
            if (j_req.contains("bursts_per_process")) {
                bursts_per_process = j_req["bursts_per_process"];
            }
        }

        // Run simulation
        vector<Process> processes = gather_real_process_data(num_processes, bursts_per_process);
        vector<SimulatorResult> sim_results = run_all_simulators(processes);

        // Build JSON response
        json j_response;
        j_response["status"] = "success";
        j_response["data"] = json::array();

        for (const auto& sr : sim_results) {
            json j_sr;
            j_sr["scheduler_name"] = sr.scheduler_name;
            j_sr["avg_waiting_time"] = sr.avg_waiting_time;
            j_sr["avg_turnaround_time"] = sr.avg_turnaround_time;
            
            json j_logs = json::array();
            for (const auto& log : sr.logs) {
                j_logs.push_back({
                    {"time", log.time},
                    {"pid", log.pid},
                    {"burst_time", log.burst_time},
                    {"waiting_time", log.waiting_time}
                });
            }
            j_sr["logs"] = j_logs;
            j_response["data"].push_back(j_sr);
        }

        cout << j_response.dump() << endl;

    } catch (const exception& e) {
        json j_err;
        j_err["status"] = "error";
        j_err["message"] = e.what();
        cout << j_err.dump() << endl;
        return 1;
    }

    return 0;
}
