#include "ServerApp.h"
#include "HttpRegistration.h"
#include "SyncService.h"
#include "Auth.h"
#include "AdminConsole.h"
#include <iostream>
#include <mutex>
#include <condition_variable>

void ServerApp::loadConfig(const std::string& path) {
    HttpRegistration::loadServerConfig(path);
    SyncService::instance().loadPeerConfig(path);
}

void ServerApp::loadAdminKey(const std::string& path) {
    AdminConsole::loadMasterKey(path);
}

void ServerApp::startServices() {
    AdminConsole::authenticate();
    Auth::loadCodes();             // <-- load from file
    AdminConsole::run();           // <-- REPL for gen/list/exit
    // then launch HttpRegistration, SyncService...
    HttpRegistration::start();
    SyncService::instance().start();
}

void ServerApp::runEventLoop() {
    std::cout << "EChat server running. Press Ctrl+C to exit..." << std::endl;
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable cv;
    cv.wait(lock);  // blocks forever
}