const { spawn } = require('child_process');
const path = require('path');

module.exports = async (req, res) => {
    // Only accept POST requests
    if (req.method !== 'POST') {
        return res.status(405).json({ error: 'Method not allowed. Use POST.' });
    }

    try {
        // Parse input JSON from frontend
        const inputStr = JSON.stringify(req.body || {});
        
        // Find the compiled C++ executable
        // Vercel puts included files in the same directory structure, or root if configured
        let executablePath = path.join(process.cwd(), 'api', 'SimulatorCLI');
        
        // If testing locally on Windows and you didn't run the build script, it might be SimulatorCLI.exe
        if (process.platform === 'win32') {
            executablePath = path.join(process.cwd(), 'SimulatorCLI.exe');
            const fs = require('fs');
            if (!fs.existsSync(executablePath)) {
                executablePath = path.join(process.cwd(), 'build', 'SimulatorCLI.exe');
            }
        }
        
        // Spawn the C++ process
        const child = spawn(executablePath, [], { shell: false });
        
        let stdoutData = '';
        let stderrData = '';
        
        // Collect stdout
        child.stdout.on('data', (data) => {
            stdoutData += data.toString();
        });
        
        // Collect stderr
        child.stderr.on('data', (data) => {
            stderrData += data.toString();
        });
        
        // Send JSON to C++ process
        child.stdin.write(inputStr);
        child.stdin.end();
        
        // Wait for C++ process to finish
        child.on('close', (code) => {
            if (code !== 0) {
                console.error("C++ process failed with code", code);
                console.error(stderrData);
                return res.status(500).json({ 
                    status: 'error', 
                    message: `C++ Core Execution Failed: ${stderrData}` 
                });
            }
            
            try {
                // Parse the JSON output from the C++ process and send back to frontend
                const result = JSON.parse(stdoutData);
                res.status(200).json(result);
            } catch (parseError) {
                console.error("Failed to parse C++ output:", stdoutData);
                res.status(500).json({ 
                    status: 'error', 
                    message: 'Failed to parse JSON output from C++ Core.' 
                });
            }
        });
        
        // Handle process spawn errors (e.g., executable not found)
        child.on('error', (err) => {
            console.error("Failed to spawn C++ process:", err);
            res.status(500).json({ 
                status: 'error', 
                message: `Failed to start C++ engine. Executable not found at ${executablePath}.` 
            });
        });

    } catch (error) {
        console.error("API Route Error:", error);
        res.status(500).json({ status: 'error', message: error.message });
    }
};
