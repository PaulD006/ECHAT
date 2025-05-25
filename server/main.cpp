#include "ServerApp.h"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ServerApp app;
        app.loadConfig("echatserv.config");
        app.loadAdminKey("admin.key");
        app.startServices();
        app.runEventLoop();
    } catch(const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}