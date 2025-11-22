#include <QApplication>

#include "window.h"
#include "PgpKeyManager.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

    //Testing PgpKeyManager functionality
    PgpKeyManager key_manager("demo@example.com");

    std::string public_key = key_manager.exportPublicKeyArmored();
    std::string secret_key = key_manager.exportSecretKeyArmored();

    std::cout << "Public key:\n" << public_key << "\n";
    std::cout << "Secret key:\n" << secret_key << "\n";

    //Testing GUI functionality
    QApplication app(argc, argv);

    Window w;
    w.show();

    return app.exec();
}
