import numpy as np
import pandas as pd
import random

def generate_process_data(num_processes=200, min_bursts=5, max_bursts=20):
    """
    Generates synthetic dataset of CPU bursts for processes simulating real-world workloads.
    """
    np.random.seed(42)
    random.seed(42)
    
    data = []
    
    for pid in range(1, num_processes + 1):
        num_bursts = random.randint(min_bursts, max_bursts)
        
        # Determine process behavior type
        # 1: CPU-bound (long bursts, low variance)
        # 2: I/O-bound (short bursts, low variance)
        # 3: Mixed/Random (high variance)
        # 4: Periodic (fluctuates up and down)
        process_type = random.choices(['CPU_BOUND', 'IO_BOUND', 'MIXED', 'PERIODIC'], weights=[0.2, 0.4, 0.3, 0.1])[0]
        
        if process_type == 'CPU_BOUND':
            base_burst = random.randint(50, 150)
            bursts = np.random.normal(base_burst, 10, num_bursts)
        elif process_type == 'IO_BOUND':
            base_burst = random.randint(2, 15)
            bursts = np.random.normal(base_burst, 2, num_bursts)
        elif process_type == 'MIXED':
            bursts = np.random.uniform(5, 100, num_bursts)
        else: # PERIODIC
            base_burst = random.randint(20, 80)
            amplitude = random.randint(10, 30)
            bursts = [base_burst + amplitude * np.sin(i) for i in range(num_bursts)]
            
        bursts = np.clip(bursts, 1, 200).astype(int) # Ensure bursts are positive and reasonably sized
        
        for seq_id, burst in enumerate(bursts):
            data.append({
                'process_id': pid,
                'process_type': process_type,
                'sequence_id': seq_id,
                'burst_time': burst
            })
            
    df = pd.DataFrame(data)
    return df

def create_ml_dataset(df, history_window=3):
    """
    Transforms sequence data into supervised ML format using a sliding window.
    Features: last N burst times
    Target: next burst time
    """
    records = []
    
    for pid, group in df.groupby('process_id'):
        bursts = group.sort_values('sequence_id')['burst_time'].values
        
        if len(bursts) <= history_window:
            continue
            
        for i in range(len(bursts) - history_window):
            features = bursts[i:i+history_window]
            target = bursts[i+history_window]
            
            record = {f'burst_minus_{history_window-j}': features[j] for j in range(history_window)}
            record['target_burst'] = target
            
            records.append(record)
            
    return pd.DataFrame(records)

if __name__ == "__main__":
    df_raw = generate_process_data(num_processes=10)
    print("Raw Process Data Sample:")
    print(df_raw.head())
    
    df_ml = create_ml_dataset(df_raw, history_window=3)
    print("\nML Dataset Sample:")
    print(df_ml.head())
    print("\nTotal ML samples generated:", len(df_ml))
