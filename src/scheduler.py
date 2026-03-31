import numpy as np

class Scheduler:
    def __init__(self):
        self.name = "Base Scheduler"
        
    def schedule(self, ready_queue, current_time):
        """
        Returns the next process to run from the ready_queue.
        """
        raise NotImplementedError

class FCFS(Scheduler):
    def __init__(self):
        self.name = "FCFS"
        
    def schedule(self, ready_queue, current_time):
        # Earliest arrival time first
        return min(ready_queue, key=lambda p: p.arrival_time)

class SJF(Scheduler):
    def __init__(self):
        self.name = "Ideal SJF"
        
    def schedule(self, ready_queue, current_time):
        # Shortest actual next burst
        return min(ready_queue, key=lambda p: p.get_next_actual_burst())

class ExponentialSJF(Scheduler):
    def __init__(self, alpha=0.5, initial_guess=10):
        self.name = f"Exponential SJF (alpha={alpha})"
        self.alpha = alpha
        self.initial_guess = initial_guess
        
    def schedule(self, ready_queue, current_time):
        # Shortest predicted burst using exponential averaging
        # First we need to ensure predictions are up to date
        for p in ready_queue:
            if not hasattr(p, 'exp_prediction'):
                p.exp_prediction = self.initial_guess
            
            # If the process just finished a burst and was added back, update its prediction
            # We track the last burst we incorporated
            last_burst_idx = getattr(p, 'last_exp_burst_idx', -1)
            current_burst_idx = p.current_burst_index
            
            if last_burst_idx < current_burst_idx - 1:
                # Need to update prediction with the most recently finished burst
                actual_last_burst = p.bursts[current_burst_idx - 1]
                p.exp_prediction = self.alpha * actual_last_burst + (1 - self.alpha) * p.exp_prediction
                p.last_exp_burst_idx = current_burst_idx - 1
                
        return min(ready_queue, key=lambda p: p.exp_prediction)

class AISJF(Scheduler):
    def __init__(self, model, history_window=3):
        self.name = "AI-Predicted SJF"
        self.model = model
        self.history_window = history_window
        
    def schedule(self, ready_queue, current_time):
        for p in ready_queue:
            history = p.get_history(self.history_window)
            if len(history) < self.history_window:
                # Not enough history, fallback to a simple average or recent burst
                p.ai_prediction = np.mean(history) if len(history) > 0 else 10
            else:
                # Predict using ML model
                features = np.array(history).reshape(1, -1)
                p.ai_prediction = self.model.predict(features)[0]
                
        return min(ready_queue, key=lambda p: p.ai_prediction)
