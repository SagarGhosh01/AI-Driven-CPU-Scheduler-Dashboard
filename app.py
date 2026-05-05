from flask import Flask, request, jsonify, render_template
import subprocess
import json
import os

app = Flask(__name__, static_folder='static', template_folder='templates')

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/simulate', methods=['POST'])
def simulate():
    try:
        # Get JSON from request
        req_data = request.get_json(silent=True) or {}
        input_str = json.dumps(req_data)
        
        # Run C++ executable
        executable = './SimulatorCLI'
        if os.name == 'nt':
            if os.path.exists('SimulatorCLI.exe'):
                executable = 'SimulatorCLI.exe'
            elif os.path.exists('build/Debug/SimulatorCLI.exe'):
                executable = 'build/Debug/SimulatorCLI.exe'
            elif os.path.exists('build/Release/SimulatorCLI.exe'):
                executable = 'build/Release/SimulatorCLI.exe'
            elif os.path.exists('build/SimulatorCLI.exe'):
                executable = 'build/SimulatorCLI.exe'
            else:
                executable = 'SimulatorCLI.exe'
                
        process = subprocess.Popen(
            [executable],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        stdout, stderr = process.communicate(input=input_str)
        
        if process.returncode != 0:
            return jsonify({"status": "error", "message": f"C++ Core Error: {stderr}"}), 500
            
        # Parse output from C++ core
        return jsonify(json.loads(stdout))
        
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port)
