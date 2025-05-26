#include <QApplication>
#include "LoginDialog.h"
#include "Storage.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ensure config directory exists & load config
    Storage::init("config_echat/");

    LoginDialog login;
    if (login.exec() != QDialog::Accepted)
        return 0;

    NetworkClient *client = login.client();
    MainWindow w(client);
    w.show();

    return app.exec();
}