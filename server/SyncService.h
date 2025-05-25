#pragma once
#include <vector>
#include <string>

class SyncService {
public:
    static SyncService& instance();
    void loadPeerConfig(const std::string& path);
    void start();
private:
    SyncService();
    std::vector<std::pair<std::string,int>> peers;
    void acceptLoop();
    void connectLoop();
};