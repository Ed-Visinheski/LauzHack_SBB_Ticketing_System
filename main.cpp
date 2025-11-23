#include <QApplication>

#include "window.h"
#include "ticketInspector.h"
#include "PgpKeyManager.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);

    // Check command line arguments
    bool inspectorMode = false;
    bool userMode = false;
    bool bothMode = false;
    
    for (int i = 1; i < argc; ++i) {
        QString arg(argv[i]);
        if (arg == "--inspector" || arg == "-i") {
            inspectorMode = true;
        } else if (arg == "--user" || arg == "-u") {
            userMode = true;
        } else if (arg == "--both" || arg == "-b") {
            bothMode = true;
        }
    }

    // If no arguments, default to user mode
    if (!inspectorMode && !userMode && !bothMode) {
        userMode = true;
    }

    // Pointers to keep windows alive
    Window* userWindow = nullptr;
    TicketInspector* inspectorWindow = nullptr;

    if (bothMode || (inspectorMode && userMode)) {
        // Launch both windows
        userWindow = new Window();
        userWindow->setWindowTitle("SBB Ticketing - User");
        userWindow->show();
        
        inspectorWindow = new TicketInspector();
        inspectorWindow->setWindowTitle("SBB Ticketing - Inspector");
        inspectorWindow->show();
        
        // Position windows side by side if possible
        QRect userGeometry = userWindow->frameGeometry();
        userGeometry.moveLeft(100);
        userWindow->move(userGeometry.topLeft());
        
        QRect inspectorGeometry = inspectorWindow->frameGeometry();
        inspectorGeometry.moveLeft(userGeometry.right() + 20);
        inspectorWindow->move(inspectorGeometry.topLeft());
        
    } else if (inspectorMode) {
        // Launch inspector window only
        inspectorWindow = new TicketInspector();
        inspectorWindow->show();
        
    } else {
        // Launch user window only
        userWindow = new Window();
        userWindow->show();
    }

    int result = app.exec();
    
    // Cleanup
    delete userWindow;
    delete inspectorWindow;
    
    return result;
}
