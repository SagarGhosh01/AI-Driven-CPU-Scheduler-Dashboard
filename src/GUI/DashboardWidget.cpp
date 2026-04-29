#include "GUI/DashboardWidget.h"
#include <QVBoxLayout>
#include <algorithm>

DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(300);
    setupChart();
}

void DashboardWidget::setupChart() {
    m_chart = new QChart();
    m_chart->setTitle("Scheduler Performance Comparison");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    
    // Set a modern dark theme to match GanttWidget
    m_chart->setTheme(QChart::ChartThemeDark);

    m_series = new QBarSeries();
    
    m_waitSet = new QBarSet("Avg Waiting Time");
    m_turnaroundSet = new QBarSet("Avg Turnaround Time");
    m_responseSet = new QBarSet("Avg Response Time");

    // Assigning distinct colors for the metrics
    m_waitSet->setColor(QColor("#e74c3c"));       // Red
    m_turnaroundSet->setColor(QColor("#f39c12")); // Orange
    m_responseSet->setColor(QColor("#3498db"));   // Blue

    m_series->append(m_waitSet);
    m_series->append(m_turnaroundSet);
    m_series->append(m_responseSet);
    
    m_chart->addSeries(m_series);

    // X Axis Setup
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_series->attachAxis(axisX);

    // Y Axis Setup
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Time (units)");
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series->attachAxis(axisY);

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_chartView);
    layout->setContentsMargins(0, 0, 0, 0);
}

void DashboardWidget::setMetrics(const std::map<std::string, SchedulerMetrics>& results) {
    // Clear old data safely
    m_waitSet->remove(0, m_waitSet->count());
    m_turnaroundSet->remove(0, m_turnaroundSet->count());
    m_responseSet->remove(0, m_responseSet->count());

    QStringList categories;
    double maxY = 0.0;

    for (const auto& [name, metrics] : results) {
        categories << QString::fromStdString(name);
        
        *m_waitSet << metrics.avgWaitingTime;
        *m_turnaroundSet << metrics.avgTurnaroundTime;
        *m_responseSet << metrics.avgResponseTime;

        // Find the absolute maximum to scale the Y axis appropriately
        double localMax = std::max({metrics.avgWaitingTime, metrics.avgTurnaroundTime, metrics.avgResponseTime});
        if (localMax > maxY) {
            maxY = localMax;
        }
    }

    // Update X Axis Categories
    auto axesX = m_chart->axes(Qt::Horizontal);
    if (!axesX.isEmpty()) {
        auto* axisX = qobject_cast<QBarCategoryAxis*>(axesX.first());
        if (axisX) {
            axisX->clear();
            axisX->append(categories);
        }
    }

    // Update Y Axis Range
    auto axesY = m_chart->axes(Qt::Vertical);
    if (!axesY.isEmpty()) {
        auto* axisY = qobject_cast<QValueAxis*>(axesY.first());
        if (axisY) {
            axisY->setRange(0, maxY * 1.1); // Add 10% padding space on top
        }
    }
}
