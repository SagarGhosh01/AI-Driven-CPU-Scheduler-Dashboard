document.addEventListener('DOMContentLoaded', () => {
    // UI Elements
    const runBtn = document.getElementById('runBtn');
    const loadingOverlay = document.getElementById('loadingOverlay');
    const emptyState = document.getElementById('emptyState');
    const dashboardGrid = document.getElementById('dashboardGrid');
    const errorBox = document.getElementById('errorBox');
    
    const kpiBestAlgo = document.getElementById('kpiBestAlgo');
    const kpiBestScore = document.getElementById('kpiBestScore');
    const kpiAIWait = document.getElementById('kpiAIWait');
    const kpiWaitDiff = document.getElementById('kpiWaitDiff');
    const kpiTotalTime = document.getElementById('kpiTotalTime');
    
    let barChartInstance = null;
    let ganttInstances = {}; // Map of scheduler name to ApexCharts instance
    let currentSimulationData = null; // Store latest results for report generation

    // Tabs logic delegated to dynamic creation in renderGanttCharts

    runBtn.addEventListener('click', async () => {
        const numProcesses = parseInt(document.getElementById('numProcesses').value) || 15;
        
        // UI Reset & Loading State
        errorBox.classList.add('hidden');
        emptyState.classList.add('hidden');
        dashboardGrid.classList.add('hidden');
        loadingOverlay.classList.remove('hidden');
        runBtn.disabled = true;
        
        try {
            const response = await fetch('/api/simulate', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ num_processes: numProcesses })
            });
            
            const result = await response.json();
            
            if (result.status === 'success') {
                processAndRenderAll(result.data);
                dashboardGrid.classList.remove('hidden');
            } else {
                throw new Error(result.message || 'Simulation failed on server.');
            }
        } catch (error) {
            errorBox.innerHTML = `<strong>Execution Failed:</strong><br>${error.message}`;
            errorBox.classList.remove('hidden');
            emptyState.classList.remove('hidden');
        } finally {
            loadingOverlay.classList.add('hidden');
            runBtn.disabled = false;
        }
    });

    function processAndRenderAll(data) {
        currentSimulationData = data;
        
        // Find the best algorithm based on Turnaround Time
        let best = data.reduce((prev, curr) => (prev.avg_turnaround_time < curr.avg_turnaround_time) ? prev : curr);
        kpiBestAlgo.textContent = best.scheduler_name;
        kpiBestScore.textContent = `${best.avg_turnaround_time.toFixed(2)} units (avg TT)`;
        
        // Find AI vs FCFS Diff
        const aiData = data.find(d => d.scheduler_name.includes('AI'));
        const fcfsData = data.find(d => d.scheduler_name.includes('FCFS'));
        
        if (aiData && fcfsData) {
            kpiAIWait.textContent = aiData.avg_waiting_time.toFixed(2);
            const diff = fcfsData.avg_waiting_time - aiData.avg_waiting_time;
            const diffPercent = ((diff / fcfsData.avg_waiting_time) * 100).toFixed(1);
            
            if (diff > 0) {
                kpiWaitDiff.innerHTML = `<span style="color:var(--success)">▼ ${Math.abs(diff).toFixed(2)} (${diffPercent}%)</span> vs FCFS`;
            } else {
                kpiWaitDiff.innerHTML = `<span style="color:var(--danger)">▲ ${Math.abs(diff).toFixed(2)} (${Math.abs(diffPercent)}%)</span> vs FCFS`;
            }
        }
        
        // Calculate Total Workload from any scheduler (they process the same dataset)
        if (data.length > 0) {
            const totalWork = data[0].logs.reduce((sum, log) => sum + log.burst_time, 0);
            kpiTotalTime.textContent = totalWork.toLocaleString();
        }

        // Render Visualizations
        renderBarChart(data);
        renderGanttCharts(data);
    }

    function renderBarChart(data) {
        const ctx = document.getElementById('barChart').getContext('2d');
        if (barChartInstance) { barChartInstance.destroy(); }
        
        // Premium FinTech themes
        const colorPrimary = '#3b82f6';
        const colorSecondary = '#8b5cf6';
        
        barChartInstance = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: data.map(d => d.scheduler_name),
                datasets: [
                    {
                        label: 'Avg Waiting Time',
                        data: data.map(d => d.avg_waiting_time),
                        backgroundColor: 'rgba(59, 130, 246, 0.85)',
                        borderWidth: 0,
                        borderRadius: 6,
                        barPercentage: 0.6,
                        categoryPercentage: 0.8
                    },
                    {
                        label: 'Avg Turnaround Time',
                        data: data.map(d => d.avg_turnaround_time),
                        backgroundColor: 'rgba(139, 92, 246, 0.85)',
                        borderWidth: 0,
                        borderRadius: 6,
                        barPercentage: 0.6,
                        categoryPercentage: 0.8
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    intersect: false,
                    mode: 'index',
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        grid: { color: '#27272a', drawBorder: false },
                        ticks: { color: '#a1a1aa', font: { family: 'JetBrains Mono' } }
                    },
                    x: {
                        grid: { display: false, drawBorder: false },
                        ticks: { color: '#a1a1aa', font: { family: 'Plus Jakarta Sans', weight: 500 } }
                    }
                },
                plugins: {
                    legend: {
                        labels: { color: '#fafafa', usePointStyle: true, boxWidth: 8, font: { family: 'Plus Jakarta Sans' } },
                        position: 'top',
                        align: 'end'
                    },
                    tooltip: {
                        backgroundColor: '#1e1e22',
                        titleColor: '#fafafa',
                        bodyColor: '#a2a2ac',
                        borderColor: '#27272a',
                        borderWidth: 1,
                        padding: 12,
                        cornerRadius: 8,
                        titleFont: { family: 'Plus Jakarta Sans', size: 14 },
                        bodyFont: { family: 'JetBrains Mono', size: 13 }
                    }
                }
            }
        });
    }

    function renderGanttCharts(data) {
        const container = document.getElementById('gantt-containers');
        const tabsContainer = document.getElementById('ganttTabs');
        
        container.innerHTML = ''; // Clear previous
        tabsContainer.innerHTML = ''; // Clear previous tabs
        
        // Define vibrant palette for PIDs (ApexCharts default palette)
        const palette = ['#008FFB', '#00E396', '#FEB019', '#FF4560', '#775DD0', '#3F51B5', '#546E7A', '#D4526E', '#8D5B4C', '#F86624', '#D7263D', '#1B998B', '#2E294E', '#F46036', '#E2C044'];
        
        data.forEach((sched, index) => {
            // Generate valid DOM ID by removing non-alphanumeric chars
            const safeName = sched.scheduler_name.replace(/[^a-zA-Z0-9]/g, '-');
            const chartDivId = `gantt-${safeName}`;
            
            // Create Tab Button
            const tabBtn = document.createElement('button');
            tabBtn.className = `tab-btn ${index === 0 ? 'active' : ''}`;
            tabBtn.textContent = sched.scheduler_name.replace(' (alpha=0.5)', ''); // simpler label
            tabBtn.setAttribute('data-target', chartDivId);
            
            tabBtn.addEventListener('click', (e) => {
                document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
                e.target.classList.add('active');
                
                document.querySelectorAll('.gantt-chart-wrapper').forEach(c => c.classList.add('gantt-hidden'));
                const targetChart = document.getElementById(chartDivId);
                if (targetChart) targetChart.classList.remove('gantt-hidden');
            });
            
            tabsContainer.appendChild(tabBtn);
            
            // Create Wrapper
            const wrapper = document.createElement('div');
            wrapper.id = chartDivId;
            wrapper.className = `gantt-chart-wrapper ${index === 0 ? '' : 'gantt-hidden'}`;
            wrapper.style.height = '350px';
            wrapper.style.width = '100%';
            container.appendChild(wrapper);

            // Group logs into series data by Process ID
            // Format for ApexCharts RangeBar: { name: 'P1', data: [{ x: 'CPU', y: [start_time, end_time] }] }
            const processMap = {};
            
            let currentGlobalTime = 0; // Using cumulative time based on logs array order for Gantt visualization
            // Reconstruct absolute timeline since we only have duration and wait time
            sched.logs.forEach(log => {
                if (!processMap[log.pid]) {
                    processMap[log.pid] = [];
                }
                
                const startTime = currentGlobalTime;
                const endTime = currentGlobalTime + log.burst_time;
                
                processMap[log.pid].push({
                    x: 'CPU Pipeline',
                    y: [startTime, endTime],
                    burstScore: log.burst_time
                });
                
                currentGlobalTime = endTime;
            });
            
            const seriesData = Object.keys(processMap).map((pid, idx) => {
                return {
                    name: `Process ${pid}`,
                    data: processMap[pid],
                    color: palette[idx % palette.length]
                };
            });

            const options = {
                series: seriesData,
                chart: {
                    type: 'rangeBar',
                    height: 350,
                    background: 'transparent',
                    toolbar: { show: true, tools: { zoom: true, pan: true, reset: true } },
                    animations: { enabled: true, easing: 'easeinout', speed: 800 }
                },
                plotOptions: {
                    bar: {
                        horizontal: true,
                        barHeight: '60%',
                        rangeBarGroupRows: true, // Pack the same process bursts into same vertical space
                        borderRadius: 4
                    }
                },
                theme: { mode: 'dark' },
                xaxis: {
                    type: 'numeric',
                    tickAmount: 10,
                    labels: {
                        formatter: function(val) { return Math.floor(val) + "ms"; },
                        style: { colors: '#a1a1aa', fontFamily: 'JetBrains Mono' }
                    },
                    axisBorder: { show: false }
                },
                yaxis: { show: false }, // Hide the 'CPU Pipeline' label to optimize space
                grid: {
                    xaxis: { lines: { show: true } },
                    yaxis: { lines: { show: false } },
                    borderColor: '#27272a'
                },
                legend: {
                    position: 'bottom',
                    horizontalAlign: 'center',
                    labels: { colors: '#fafafa', fontFamily: 'Plus Jakarta Sans' },
                    markers: { radius: 4 }
                },
                tooltip: {
                    theme: 'dark',
                    custom: function({series, seriesIndex, dataPointIndex, w}) {
                        const data = w.globals.initialSeries[seriesIndex].data[dataPointIndex];
                        const processName = w.globals.initialSeries[seriesIndex].name;
                        const duration = data.y[1] - data.y[0];
                        
                        return `
                            <div style="padding: 10px; background: #121214; border: 1px solid #3f3f46; border-radius: 6px; box-shadow: 0 4px 6px rgba(0,0,0,0.5);">
                                <strong style="color: ${w.globals.colors[seriesIndex]}; font-family: 'Plus Jakarta Sans';">${processName}</strong><br>
                                <span style="font-family: 'JetBrains Mono'; font-size: 13px; color: #a1a1aa;">Burst: ${duration} units</span><br>
                                <span style="font-family: 'JetBrains Mono'; font-size: 13px; color: #a1a1aa;">Timeline: T+${data.y[0]} to T+${data.y[1]}</span>
                            </div>
                        `;
                    }
                }
            };
            
            // Clean up old instance if exists
            if (ganttInstances[sched.scheduler_name]) {
                ganttInstances[sched.scheduler_name].destroy();
            }
            
            const chart = new ApexCharts(document.querySelector(`#${chartDivId}`), options);
            chart.render();
            ganttInstances[sched.scheduler_name] = chart;
        });
    }

    // Report Generation Logic
    const downloadReportBtn = document.getElementById('downloadReportBtn');
    if (downloadReportBtn) {
        downloadReportBtn.addEventListener('click', () => {
            if (!currentSimulationData || currentSimulationData.length === 0) {
                alert('Please execute a simulation first to generate a report.');
                return;
            }
            
            let report = "========================================================================\n";
            report += "                  NEXUS OS - CPU SIMULATION REPORT                    \n";
            report += "========================================================================\n\n";
            
            report += "--- ABOUT THIS REPORT ---\n";
            report += "This document provides a detailed breakdown of how different CPU scheduling\n";
            report += "algorithms (Heuristics) handled the exact same queue of system processes.\n\n";
            report += "TERMINOLOGY:\n";
            report += "- Waiting Time: Total time a process spent idle in the ready queue.\n";
            report += "- Turnaround Time: Total time taken from process arrival to final completion.\n";
            report += "- Burst Time: The actual amount of CPU processing time consumed.\n\n";
            
            // Calculate best
            let best = currentSimulationData.reduce((prev, curr) => (prev.avg_turnaround_time < curr.avg_turnaround_time) ? prev : curr);
            
            report += "========================================================================\n";
            report += "                         KEY TAKEAWAYS                                \n";
            report += "========================================================================\n";
            report += `The most efficient algorithm for this workload was: **${best.scheduler_name}**.\n`;
            report += "Lower wait and turnaround times indicate better system responsiveness.\n\n";
            
            report += "PERFORMANCE SUMMARY (All Algorithms):\n";
            report += "------------------------------------------------------------------------\n";
            currentSimulationData.forEach(sched => {
                report += `[ ${sched.scheduler_name} ]\n`;
                report += `  -> Average Waiting Time:     ${sched.avg_waiting_time.toFixed(2)} units\n`;
                report += `  -> Average Turnaround Time:  ${sched.avg_turnaround_time.toFixed(2)} units\n\n`;
            });
            
            report += "========================================================================\n";
            report += "                   DETAILED PROCESS LOGS (Timeline)                     \n";
            report += "========================================================================\n";
            report += "Below is the exact chronological sequence of CPU execution for each algorithm.\n";
            
            currentSimulationData.forEach(sched => {
                report += `\n########################################################################\n`;
                report += `  ALGORITHM: ${sched.scheduler_name}\n`;
                report += `########################################################################\n`;
                
                // Add column headers with exact spacing
                report += `| ${"Time".padEnd(8)} | ${"PID".padEnd(10)} | ${"Burst Time".padEnd(12)} | ${"Accumulated Wait".padEnd(18)} |\n`;
                report += `|----------|------------|--------------|--------------------|\n`;
                
                sched.logs.forEach(log => {
                    let timeStr = `T+${log.time}`.padEnd(8);
                    let pidStr = `Process ${log.pid}`.padEnd(10);
                    let burstStr = `${log.burst_time} units`.padEnd(12);
                    let waitStr = `${log.waiting_time} units`.padEnd(18);
                    
                    report += `| ${timeStr} | ${pidStr} | ${burstStr} | ${waitStr} |\n`;
                });
            });
            
            const blob = new Blob([report], { type: 'text/plain' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `nexus_os_simulation_report_${new Date().getTime()}.txt`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        });
    }
});
