#pragma once
#include <string>

namespace AdminConsole {
    void loadMasterKey(const std::string& path);
    void authenticate();
    void run();
}