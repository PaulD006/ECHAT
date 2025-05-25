#include "ServerApp.h"
#include "HttpRegistration.h"
#include "SyncService.h"
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
    // First authenticate admin for console
    AdminConsole::authenticate();
    // Launch HTTP registration service (TLS)
    HttpRegistration::start();
    // Launch database sync and client-listener
    SyncService::instance().start();
}

void ServerApp::runEventLoop() {
    std::cout << "EChat server running. Press Ctrl+C to exit..." << std::endl;
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable cv;
    cv.wait(lock);  // blocks forever
}