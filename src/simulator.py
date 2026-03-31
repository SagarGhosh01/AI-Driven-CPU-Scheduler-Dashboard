import pandas as pd
import matplotlib.pyplot as plt

class Process:
    def __init__(self, pid, bursts):
        self.pid = pid
        self.bursts = bursts  # List of CPU burst times
        self.current_burst_index = 0
        self.arrival_time = 0
        self.history = []
        
        # Metrics
        self.waiting_times = []
        self.turnaround_times = []
        
    def get_next_actual_burst(self):
        return self.bursts[self.current_burst_index]
        
    def get_history(self, window):
        """Returns the last `window` burst times"""
        start = max(0, self.current_burst_index - window)
        return self.bursts[start:self.current_burst_index]
        
    def is_finished(self):
        return self.current_burst_index >= len(self.bursts)
        
    def complete_current_burst(self, current_time, waiting_time):
        burst_time = self.bursts[self.current_burst_index]
        self.waiting_times.append(waiting_time)
        self.turnaround_times.append(waiting_time + burst_time)
        self.history.append(burst_time)
        self.current_burst_index += 1
        return burst_time

class Simulator:
    def __init__(self, processes, scheduler, io_delay=10):
        # processes is a list of Process objects
        self.processes = processes
        self.scheduler = scheduler
        self.io_delay = io_delay
        
        self.current_time = 0
        self.ready_queue = []
        self.io_queue = [] # List of tuples: (process, io_completion_time)
        
        # initialize ready queue
        for p in self.processes:
            if p.arrival_time <= 0:
                self.ready_queue.append(p)
            else:
                self.io_queue.append((p, p.arrival_time))
            
        self.logs = []
        
    def run(self):
        print(f"\n--- Starting Simulation with {self.scheduler.name} ---")
        
        while self.ready_queue or self.io_queue:
            # Move processes from I/O queue to ready queue if I/O is done
            moved_from_io = []
            for p, completion_time in self.io_queue:
                if completion_time <= self.current_time:
                    p.arrival_time = completion_time
                    self.ready_queue.append(p)
                    moved_from_io.append((p, completion_time))
                    
            for item in moved_from_io:
                self.io_queue.remove(item)
                
            if not self.ready_queue:
                # CPU is idle, advance time to next I/O completion
                if self.io_queue:
                    next_time = min(completion_time for p, completion_time in self.io_queue)
                    self.current_time = next_time
                continue
                
            # Select next process
            selected_process = self.scheduler.schedule(self.ready_queue, self.current_time)
            self.ready_queue.remove(selected_process)
            
            # Execute burst
            waiting_time = self.current_time - selected_process.arrival_time
            burst_time = selected_process.complete_current_burst(self.current_time, waiting_time)
            
            # Log execution
            self.logs.append({
                'time': self.current_time,
                'pid': selected_process.pid,
                'burst_time': burst_time,
                'waiting_time': waiting_time
            })
            
            # Advance time
            self.current_time += burst_time
            
            # Send to I/O if not finished
            if not selected_process.is_finished():
                io_completion = self.current_time + self.io_delay
                self.io_queue.append((selected_process, io_completion))
                
        # Calculate summary metrics
        total_waiting_time = 0
        total_turnaround_time = 0
        num_bursts = 0
        
        for p in self.processes:
            total_waiting_time += sum(p.waiting_times)
            total_turnaround_time += sum(p.turnaround_times)
            num_bursts += len(p.waiting_times)
            
        avg_waiting_time = total_waiting_time / num_bursts if num_bursts > 0 else 0
        avg_turnaround_time = total_turnaround_time / num_bursts if num_bursts > 0 else 0
        
        print(f"Simulation Complete. Avg Waiting Time: {avg_waiting_time:.2f}, Avg Turnaround Time: {avg_turnaround_time:.2f}")
        return {
            'scheduler_name': self.scheduler.name,
            'avg_waiting_time': avg_waiting_time,
            'avg_turnaround_time': avg_turnaround_time
        }
