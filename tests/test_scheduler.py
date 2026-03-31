import pytest
from src.scheduler import FCFS, SJF, Scheduler
from src.simulator import Process, Simulator

def test_fcfs():
    p1 = Process(1, [10])
    p1.arrival_time = 0
    p2 = Process(2, [5])
    p2.arrival_time = 5
    
    scheduler = FCFS()
    sim = Simulator([p1, p2], scheduler, io_delay=0)
    res = sim.run()
    
    # p1 runs 0 to 10. avg waiting time = 0. Turnaround = 10
    # p2 arrives at 5, starts at 10. finishes at 15. Waiting time = 10-5 = 5. Turnaround = 10
    # Average waiting time = (0 + 5) / 2 = 2.5
    assert res['avg_waiting_time'] == 2.5
    assert res['avg_turnaround_time'] == 10.0

def test_sjf():
    p1 = Process(1, [10])
    p1.arrival_time = 0
    p2 = Process(2, [5])
    p2.arrival_time = 0
    
    scheduler = SJF()
    sim = Simulator([p1, p2], scheduler, io_delay=0)
    res = sim.run()
    
    # p2 has shorter burst (5) than p1 (10), and both arrive at 0.
    # So p2 runs 0 to 5. Waiting = 0. Turnaround = 5.
    # Then p1 runs 5 to 15. Waiting = 5. Turnaround = 15.
    # Avg waiting = 2.5, Avg turnaround = 10.0
    assert res['avg_waiting_time'] == 2.5
    assert res['avg_turnaround_time'] == 10.0
