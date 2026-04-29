#pragma once

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <map>
#include <string>

class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    ~DashboardWidget() override = default;

    struct SchedulerMetrics {
        double avgWaitingTime = 0.0;
        double avgTurnaroundTime = 0.0;
        double avgResponseTime = 0.0;
    };

    // Update the chart with new results
    // The map key should be the name of the scheduler (e.g. "FCFS", "ML Scheduler")
    void setMetrics(const std::map<std::string, SchedulerMetrics>& results);

private:
    void setupChart();

    QChart* m_chart;
    QChartView* m_chartView;
    QBarSeries* m_series;
    
    QBarSet* m_waitSet;
    QBarSet* m_turnaroundSet;
    QBarSet* m_responseSet;
};
