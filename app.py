import os
from flask import Flask, render_template, request, jsonify
from flask_cors import CORS
import pandas as pd

# Import simulation modules
from src.dataset import generate_process_data
from src.model import load_trained_model, train_and_save_model
from src.scheduler import FCFS, SJF, ExponentialSJF, AISJF
from src.simulator import Simulator, Process

app = Flask(__name__)
CORS(app)

# Ensure ML model is loaded or trained at startup so the API is fast
MODEL_PATH = 'model.pkl'
if not os.path.exists(MODEL_PATH):
    print("Training initial model...")
    train_and_save_model(num_processes=500, history_window=3, save_path=MODEL_PATH)
model = load_trained_model(MODEL_PATH)

def create_processes_from_df(df):
    processes = []
    for pid, group in df.groupby('process_id'):
        bursts = group.sort_values('sequence_id')['burst_time'].tolist()
        processes.append(Process(pid=pid, bursts=bursts))
    return processes

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/simulate', methods=['POST'])
def simulate():
    try:
        req_data = request.json or {}
        num_processes = req_data.get('num_processes', 20)
        
        # 1. Generate data
        df_eval = generate_process_data(num_processes=num_processes)
        
        # 2. Setup schedulers
        schedulers = [
            FCFS(),
            SJF(),
            ExponentialSJF(alpha=0.5),
            AISJF(model=model, history_window=3)
        ]
        
        results = []
        
        # 3. Simulate and collect detailed logs
        for sched in schedulers:
            processes = create_processes_from_df(df_eval)
            sim = Simulator(processes=processes, scheduler=sched, io_delay=10)
            res_summary = sim.run()
            
            results.append({
                'scheduler_name': res_summary['scheduler_name'],
                'avg_waiting_time': res_summary['avg_waiting_time'],
                'avg_turnaround_time': res_summary['avg_turnaround_time'],
                'logs': sim.logs  # Process timeline info for Gantt chart
            })
            
        return jsonify({
            'status': 'success',
            'data': results
        })
        
    except Exception as e:
        import traceback
        return jsonify({
            'status': 'error',
            'message': str(e),
            'trace': traceback.format_exc()
        }), 500

if __name__ == '__main__':
    # Run the Flask app
    app.run(debug=True, port=5000)
