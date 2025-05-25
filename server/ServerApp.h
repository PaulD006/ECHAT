#pragma once
#include <string>

class ServerApp {
public:
    void loadConfig(const std::string& path);
    void loadAdminKey(const std::string& path);
    void startServices();
    void runEventLoop();
};