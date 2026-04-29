#include <QApplication>
#include "GUI/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set some application-wide metadata
    QApplication::setApplicationName("AI CPU Scheduler");
    QApplication::setApplicationVersion("1.0.0");

    MainWindow window;
    window.show();
    
    return app.exec();
}
