#include <QApplication>

#include "window.h"
#include "PgpKeyManager.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    
    QApplication app(argc, argv);

    Window w;
    w.show();

    return app.exec();
}
