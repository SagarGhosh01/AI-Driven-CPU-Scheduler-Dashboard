from flask import Flask, request, jsonify
import random

app = Flask(__name__)

class Process:
    def __init__(self, p_id, bursts):
        self.pid = p_id
        self.bursts = bursts
        self.current_burst_index = 0
        self.arrival_time = 0
        self.exp_predicted_burst = 10.0
        self.last_exp_burst_idx = -1
        self.waiting_times = []
        self.turnaround_times = []

    def get_next_actual_burst(self):
        if self.current_burst_index < len(self.bursts):
            return self.bursts[self.current_burst_index]
        return 0

    def get_current_ai_prediction(self):
        if self.current_burst_index == 0:
            return 10.0
        if self.current_burst_index == 1:
            return self.bursts[0]

        history_window = 3
        n = min(self.current_burst_index, history_window)
        
        sum_x = sum(range(n))
        sum_y = 0
        sum_xy = 0
        sum_xx = sum(i * i for i in range(n))
        
        for j in range(n):
            idx = self.current_burst_index - n + j
            sum_y += self.bursts[idx]
            sum_xy += j * self.bursts[idx]
            
        denominator = (n * sum_xx - sum_x * sum_x)
        if denominator == 0:
            return self.bursts[self.current_burst_index - 1]
            
        m = (n * sum_xy - sum_x * sum_y) / denominator
        c = (sum_y - m * sum_x) / n
        
        prediction = m * n + c
        return max(1.0, prediction)

    def is_finished(self):
        return self.current_burst_index >= len(self.bursts)

    def complete_current_burst(self, current_time, waiting_time):
        burst_time = self.bursts[self.current_burst_index]
        self.waiting_times.append(waiting_time)
        self.turnaround_times.append(waiting_time + burst_time)
        self.current_burst_index += 1
        return burst_time

class Scheduler:
    def __init__(self, name):
        self.name = name
    def schedule(self, ready_queue, current_time):
        pass

class FCFS(Scheduler):
    def __init__(self):
        super().__init__("FCFS")
    def schedule(self, ready_queue, current_time):
        return min(ready_queue, key=lambda p: p.arrival_time)

class SJF(Scheduler):
    def __init__(self):
        super().__init__("Ideal SJF")
    def schedule(self, ready_queue, current_time):
        return min(ready_queue, key=lambda p: p.get_next_actual_burst())

class ExponentialSJF(Scheduler):
    def __init__(self, alpha=0.5):
        super().__init__("Exponential SJF (alpha=0.5)")
        self.alpha = alpha
    def schedule(self, ready_queue, current_time):
        for p in ready_queue:
            if p.last_exp_burst_idx < p.current_burst_index - 1:
                actual_last_burst = p.bursts[p.current_burst_index - 1]
                p.exp_predicted_burst = self.alpha * actual_last_burst + (1.0 - self.alpha) * p.exp_predicted_burst
                p.last_exp_burst_idx = p.current_burst_index - 1
        return min(ready_queue, key=lambda p: p.exp_predicted_burst)

class AISJF(Scheduler):
    def __init__(self):
        super().__init__("AI-Predicted SJF")
    def schedule(self, ready_queue, current_time):
        return min(ready_queue, key=lambda p: p.get_current_ai_prediction())

class SimulatorEngine:
    def __init__(self, processes, scheduler, io_delay=10):
        # Deep copy to ensure schedulers don't mutate the same process states
        self.processes = []
        for p in processes:
            new_p = Process(p.pid, list(p.bursts))
            new_p.arrival_time = p.arrival_time
            self.processes.append(new_p)
            
        self.scheduler = scheduler
        self.io_delay = io_delay

    def run(self):
        current_time = 0
        ready_queue = []
        io_queue = []
        
        for p in self.processes:
            p.arrival_time = 0
            ready_queue.append(p)
            
        logs = []
        
        while len(ready_queue) > 0 or len(io_queue) > 0:
            remaining_io = []
            for item in io_queue:
                p, io_time = item
                if io_time <= current_time:
                    p.arrival_time = io_time
                    ready_queue.append(p)
                else:
                    remaining_io.append(item)
            io_queue = remaining_io
            
            if len(ready_queue) == 0:
                if len(io_queue) > 0:
                    current_time = min(io_queue, key=lambda x: x[1])[1]
                continue
                
            selected = self.scheduler.schedule(ready_queue, current_time)
            ready_queue.remove(selected)
            
            waiting_time = current_time - selected.arrival_time
            burst_time = selected.complete_current_burst(current_time, waiting_time)
            
            logs.append({
                "time": current_time,
                "pid": selected.pid,
                "burst_time": burst_time,
                "waiting_time": waiting_time
            })
            current_time += burst_time
            
            if not selected.is_finished():
                io_completion = current_time + self.io_delay
                io_queue.append((selected, io_completion))
                
        total_waiting_time = sum(sum(p.waiting_times) for p in self.processes)
        total_turnaround_time = sum(sum(p.turnaround_times) for p in self.processes)
        num_bursts = sum(len(p.waiting_times) for p in self.processes)
            
        avg_waiting_time = total_waiting_time / num_bursts if num_bursts > 0 else 0
        avg_turnaround_time = total_turnaround_time / num_bursts if num_bursts > 0 else 0
        
        return {
            "scheduler_name": self.scheduler.name,
            "avg_waiting_time": avg_waiting_time,
            "avg_turnaround_time": avg_turnaround_time,
            "logs": logs
        }

def run_all_simulators(processes):
    schedulers = [FCFS(), SJF(), ExponentialSJF(0.5), AISJF()]
    results = []
    for sched in schedulers:
        sim = SimulatorEngine(processes, sched, 10)
        results.append(sim.run())
    return results

def gather_real_process_data(num_processes, bursts_per_process):
    results = []
    for i in range(num_processes):
        bursts = [random.randint(1, 100) for _ in range(bursts_per_process)]
        p = Process(i + 1, bursts)
        p.arrival_time = random.randint(0, 50)
        results.append(p)
        
    results.sort(key=lambda x: x.arrival_time)
    return results

@app.route('/api/simulate', methods=['POST'])
def simulate():
    try:
        req_data = request.get_json(silent=True) or {}
        num_processes = int(req_data.get('num_processes', 20))
        bursts_per_process = 10
        
        processes = gather_real_process_data(num_processes, bursts_per_process)
        sim_results = run_all_simulators(processes)
        
        return jsonify({
            "status": "success",
            "data": sim_results
        })
        
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500
