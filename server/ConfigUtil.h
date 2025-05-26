#pragma once
#include <string>
#include <vector>

namespace ConfigUtil {
    // Load the peer lines (skips comments and blank lines until [ssl])
    std::vector<std::string> loadPeerList(const std::string& configPath);

    // Rewrite the peer list in-place
    bool savePeerList(const std::string& configPath, const std::vector<std::string>& peers);

    // Helpers:
    bool addPeer(const std::string& configPath, const std::string& peer);
    bool removePeer(const std::string& configPath, const std::string& peer);

    // Update the SSL certificate paths under [ssl]
    bool setSslPaths(const std::string& configPath,
                     const std::string& certChain,
                     const std::string& privKey);
}