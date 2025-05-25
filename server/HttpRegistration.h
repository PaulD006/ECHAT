#pragma once
#include <string>

namespace HttpRegistration {
    void loadServerConfig(const std::string& configPath);
    void start();
}