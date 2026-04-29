#pragma once

#include <QWidget>
#include <vector>
#include "ML/Process.h"

class GanttWidget : public QWidget {
    Q_OBJECT

public:
    explicit GanttWidget(QWidget *parent = nullptr);
    ~GanttWidget() override = default;

    // Receives a list of scheduled processes to visualize
    void setProcesses(const std::vector<Process>& processes);

protected:
    // Overridden to custom draw the Gantt chart using QPainter
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<Process> m_processes;
    double m_maxTime;
    
    // Helper to pick a distinct color based on PID
    QColor getColorForPid(int pid) const;
};
