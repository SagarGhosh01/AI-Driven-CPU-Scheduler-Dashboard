#include "GUI/GanttWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

GanttWidget::GanttWidget(QWidget *parent) : QWidget(parent), m_maxTime(0.0) {
    // Set a reasonable minimum size so it doesn't collapse in the layout
    setMinimumHeight(150);
}

void GanttWidget::setProcesses(const std::vector<Process>& processes) {
    m_processes = processes;
    m_maxTime = 0.0;
    
    // Find the absolute maximum completion time to scale the drawing horizontally
    for (const auto& p : m_processes) {
        if (p.completionTime > m_maxTime) {
            m_maxTime = p.completionTime;
        }
    }
    
    // Trigger a UI repaint
    update();
}

QColor GanttWidget::getColorForPid(int pid) const {
    // Generate a pseudo-random but consistent hue based on PID.
    // The Golden Ratio conjugate (0.618033988749895) helps distribute colors evenly.
    int hue = static_cast<int>((pid * 0.618033988749895) * 360) % 360;
    return QColor::fromHsv(hue, 180, 220); // High saturation, high value for vibrant UI
}

void GanttWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int width = this->width();
    int height = this->height();
    
    // Draw modern dark background
    painter.fillRect(0, 0, width, height, QColor("#1e1e1e"));
    
    if (m_processes.empty() || m_maxTime <= 0.0) {
        painter.setPen(QColor("#888888"));
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "Run a simulation to view the Gantt Chart");
        return;
    }
    
    // Margins for padding
    int marginX = 20;
    int marginY = 40; 
    int drawWidth = width - 2 * marginX;
    int drawHeight = height - 2 * marginY;
    
    // Calculate scale: pixels per unit of time
    double pixelsPerTime = static_cast<double>(drawWidth) / m_maxTime;
    
    // Draw timeline axis
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(marginX, height - marginY + 10, width - marginX, height - marginY + 10);
    
    painter.setPen(Qt::white);
    painter.drawText(marginX - 10, height - marginY + 30, "0");
    painter.drawText(width - marginX - 10, height - marginY + 30, QString::number(m_maxTime, 'f', 1));

    // Draw each process block
    for (const auto& p : m_processes) {
        // Compute geometry
        int x = marginX + static_cast<int>(p.startTime * pixelsPerTime);
        int w = static_cast<int>((p.completionTime - p.startTime) * pixelsPerTime);
        int y = marginY;
        int h = drawHeight;
        
        // Ensure minimum 1-pixel width for extremely fast processes
        if (w < 1) w = 1;
        
        QRect blockRect(x, y, w, h);
        
        QColor baseColor = getColorForPid(p.pid);
        
        // Use an alpha channel so overlapping processes (e.g. in Round Robin macro-blocks)
        // blend together, hinting at context switching without requiring segment-level data.
        baseColor.setAlpha(160); 
        painter.fillRect(blockRect, baseColor);
        
        // Draw border
        painter.setPen(QPen(QColor(255, 255, 255, 100), 1));
        painter.drawRect(blockRect);
        
        // Draw PID text if the block is wide enough
        if (w > 25) {
            painter.setPen(Qt::white);
            QFont boldFont = painter.font();
            boldFont.setBold(true);
            painter.setFont(boldFont);
            painter.drawText(blockRect, Qt::AlignCenter, QString("P%1").arg(p.pid));
        }
    }
}
