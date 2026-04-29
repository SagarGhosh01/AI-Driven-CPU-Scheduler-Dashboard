#pragma once

#include <QMainWindow>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <map>
#include <string>
#include "GUI/GanttWidget.h"
#include "GUI/DashboardWidget.h"




class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void runSimulation();
    void exportToCsv();

private:
    void setupUi();

    // Config Panel Controls
    QSpinBox* processCountSpinBox;
    QSpinBox* historySizeSpinBox;
    QSpinBox* quantumSpinBox;
    QComboBox* workloadComboBox;
    QPushButton* runButton;
    QPushButton* exportButton;
    
    // Visualization
    GanttWidget* ganttChart;
    DashboardWidget* dashboardChart;

    // Store latest results for export
    std::map<std::string, std::vector<Process>> m_latestResults;
};
