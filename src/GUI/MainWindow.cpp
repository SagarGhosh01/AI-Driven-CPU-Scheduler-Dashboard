#include "GUI/MainWindow.h"
#include "ML/ProcessGenerator.h"
#include "ML/Predictor.h"
#include "Scheduler/FCFSScheduler.h"
#include "Scheduler/SJFScheduler.h"
#include "Scheduler/RoundRobinScheduler.h"
#include "Scheduler/MLScheduler.h"
#include "Scheduler/MetricsEngine.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <memory>



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("AI Predictive CPU Scheduler");
    resize(1024, 768); // Use a nice wide resolution
    setupUi();
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // --- Configuration Panel ---
    QHBoxLayout* configLayout = new QHBoxLayout();

    // Process Count
    configLayout->addWidget(new QLabel("Processes:"));
    processCountSpinBox = new QSpinBox();
    processCountSpinBox->setRange(1, 1000);
    processCountSpinBox->setValue(10);
    configLayout->addWidget(processCountSpinBox);

    // History Size (for ML training data per process)
    configLayout->addWidget(new QLabel("History Size:"));
    historySizeSpinBox = new QSpinBox();
    historySizeSpinBox->setRange(1, 100);
    historySizeSpinBox->setValue(10);
    configLayout->addWidget(historySizeSpinBox);

    // RR Quantum
    configLayout->addWidget(new QLabel("RR Quantum:"));
    quantumSpinBox = new QSpinBox();
    quantumSpinBox->setRange(1, 100);
    quantumSpinBox->setValue(4);
    configLayout->addWidget(quantumSpinBox);

    // Workload Profile
    configLayout->addWidget(new QLabel("Workload:"));
    workloadComboBox = new QComboBox();
    // Storing enum values directly in the Qt UserRole
    workloadComboBox->addItem("Mixed", static_cast<int>(ProcessGenerator::WorkloadType::MIXED));
    workloadComboBox->addItem("CPU Bound", static_cast<int>(ProcessGenerator::WorkloadType::CPU_BOUND));
    workloadComboBox->addItem("IO Bound", static_cast<int>(ProcessGenerator::WorkloadType::IO_BOUND));
    configLayout->addWidget(workloadComboBox);

    mainLayout->addLayout(configLayout);

    // --- Action Buttons ---
    QHBoxLayout* actionLayout = new QHBoxLayout();
    
    runButton = new QPushButton("Execute Simulation");
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runSimulation);
    actionLayout->addWidget(runButton);

    exportButton = new QPushButton("Export to CSV");
    exportButton->setEnabled(false); // Disabled until a simulation runs
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportToCsv);
    actionLayout->addWidget(exportButton);

    mainLayout->addLayout(actionLayout);

    // --- Visualizations ---
    QLabel* ganttLabel = new QLabel("<b>Gantt Chart (Timeline)</b>");
    ganttLabel->setFont(QFont("Arial", 12));
    mainLayout->addWidget(ganttLabel);
    
    ganttChart = new GanttWidget();
    mainLayout->addWidget(ganttChart);

    QLabel* dashboardLabel = new QLabel("<b>Performance Metrics</b>");
    dashboardLabel->setFont(QFont("Arial", 12));
    mainLayout->addWidget(dashboardLabel);

    dashboardChart = new DashboardWidget();
    mainLayout->addWidget(dashboardChart);
}

void MainWindow::runSimulation() {
    int numProcesses = processCountSpinBox->value();
    int historySize = historySizeSpinBox->value();
    double quantum = static_cast<double>(quantumSpinBox->value());
    auto workload = static_cast<ProcessGenerator::WorkloadType>(workloadComboBox->currentData().toInt());

    try {
        // 1. Generate Historical Training Data & Train ML Predictor
        // We generate a separate batch of processes acting as past execution logs to train the model.
        auto trainingProcesses = ProcessGenerator::generate(200, historySize, workload);
        auto predictor = std::make_shared<Predictor>();
        predictor->train(trainingProcesses);

        // 2. Generate actual processes for the live simulation
        auto simulationProcesses = ProcessGenerator::generate(numProcesses, historySize, workload);

        // 3. Initialize Schedulers
        FCFSScheduler fcfs;
        SJFScheduler sjf;
        RoundRobinScheduler rr(quantum);
        MLScheduler ml(predictor);

        // 4. Execute Schedulers independently on the same simulation batch
        auto fcfsResult = fcfs.schedule(simulationProcesses);
        auto sjfResult = sjf.schedule(simulationProcesses);
        auto rrResult = rr.schedule(simulationProcesses);
        auto mlResult = ml.schedule(simulationProcesses);

        // 5. Compute performance metrics
        MetricsEngine::computeMetrics(fcfsResult);
        MetricsEngine::computeMetrics(sjfResult);
        MetricsEngine::computeMetrics(rrResult);
        MetricsEngine::computeMetrics(mlResult);

        // 6. Aggregate results for the Dashboard Widget
        std::map<std::string, DashboardWidget::SchedulerMetrics> metricsMap;
        
        metricsMap["FCFS"] = {
            MetricsEngine::getAverageWaitingTime(fcfsResult),
            MetricsEngine::getAverageTurnaroundTime(fcfsResult),
            MetricsEngine::getAverageResponseTime(fcfsResult)
        };
        metricsMap["SJF"] = {
            MetricsEngine::getAverageWaitingTime(sjfResult),
            MetricsEngine::getAverageTurnaroundTime(sjfResult),
            MetricsEngine::getAverageResponseTime(sjfResult)
        };
        metricsMap["Round Robin"] = {
            MetricsEngine::getAverageWaitingTime(rrResult),
            MetricsEngine::getAverageTurnaroundTime(rrResult),
            MetricsEngine::getAverageResponseTime(rrResult)
        };
        metricsMap["ML Scheduler"] = {
            MetricsEngine::getAverageWaitingTime(mlResult),
            MetricsEngine::getAverageTurnaroundTime(mlResult),
            MetricsEngine::getAverageResponseTime(mlResult)
        };

        dashboardChart->setMetrics(metricsMap);

        // 7. Store results and update UI
        m_latestResults.clear();
        m_latestResults["FCFS"] = fcfsResult;
        m_latestResults["SJF"] = sjfResult;
        m_latestResults["Round Robin"] = rrResult;
        m_latestResults["ML Scheduler"] = mlResult;
        
        exportButton->setEnabled(true);

        // Visualize the ML Scheduler's timeline by default
        ganttChart->setProcesses(mlResult);

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Simulation Error", 
            QString("An error occurred during simulation:\n%1").arg(e.what()));
    }
}

void MainWindow::exportToCsv() {
    if (m_latestResults.empty()) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Export Simulation Data", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);
    // Write header
    out << "Scheduler,PID,ArrivalTime,ActualBurst,PredictedBurst,StartTime,CompletionTime,WaitingTime,TurnaroundTime,ResponseTime\n";

    for (const auto& [schedulerName, processes] : m_latestResults) {
        for (const auto& p : processes) {
            out << QString::fromStdString(schedulerName) << ","
                << p.pid << ","
                << p.arrivalTime << ","
                << p.actualBurst << ","
                << p.predictedBurst << ","
                << p.startTime << ","
                << p.completionTime << ","
                << p.waitingTime << ","
                << p.turnaroundTime << ","
                << p.responseTime << "\n";
        }
    }

    file.close();
    QMessageBox::information(this, "Export Complete", "Simulation data exported successfully!");
}
