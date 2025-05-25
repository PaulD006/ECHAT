#include "SyncService.h"
#include "UserDB.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <thread>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace asio = boost::asio;

SyncService& SyncService::instance() {
    static SyncService inst;
    return inst;
}

SyncService::SyncService() {}

void SyncService::loadPeerConfig(const std::string& path) {
    std::ifstream cfg(path);
    if (!cfg) {
        std::cerr << "SyncService: cannot open config file " << path << "\n";
        return;
    }

    std::string line;
    while (std::getline(cfg, line)) {
        const auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            continue;
        const auto last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, last - first + 1);

        if (line.empty() || line[0] == '#' || line[0] == '[')
            continue;

        auto pos = line.find(':');
        if (pos == std::string::npos)
            continue;

        std::string host = line.substr(0, pos);
        std::string portStr = line.substr(pos + 1);
        const auto portLast = portStr.find_last_not_of(" \t\r\n");
        portStr = portStr.substr(0, portLast + 1);

        int port = 0;
        try {
            port = std::stoi(portStr);
        } catch (const std::exception& e) {
            std::cerr << "SyncService: invalid port '" << portStr
                      << "' in line: " << line << "\n";
            continue;
        }

        peers.emplace_back(host, port);
    }
}

void SyncService::start() {
    std::thread(&SyncService::acceptLoop, this).detach();
    std::thread(&SyncService::connectLoop, this).detach();
}

void SyncService::acceptLoop() {
    try {
        asio::io_context ioc;
        ssl::context ctx(ssl::context::tlsv12);
        ctx.use_certificate_chain_file("/home/ubuntu/ECHAT/certs/server.crt");
        ctx.use_private_key_file("/home/ubuntu/ECHAT/certs/server.key", ssl::context::pem);
        tcp::acceptor acceptor{ioc, {tcp::v4(), 9001}};
        for (;;) {
            ssl::stream<tcp::socket> sock{ioc, ctx};
            acceptor.accept(sock.next_layer());
            sock.handshake(ssl::stream_base::server);
            std::thread([s = std::move(sock)]() mutable {
                try {
                    for (;;) {
                        uint32_t len;
                        asio::read(s, asio::buffer(&len, 4));
                        len = ntohl(len);
                        std::vector<char> buf(len);
                        asio::read(s, asio::buffer(buf));
                        UserRecord rec;
                        std::istringstream is(std::string(buf.begin(), buf.end()));
                        is >> rec.username >> rec.hash >> rec.timestamp;
                        UserDB::instance().mergeUser(rec);
                    }
                } catch (...) {
                }
            }).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Sync accept error: " << e.what() << "\n";
    }
}

void SyncService::connectLoop() {
    for (auto& p : peers) {
        std::thread([host = p.first, port = p.second] {
            try {
                asio::io_context ioc;
                ssl::context ctx(ssl::context::tlsv12_client);
                ctx.load_verify_file("/home/ubuntu/ECHAT/certs/server.crt");
                ssl::stream<tcp::socket> sock{ioc, ctx};
                tcp::resolver resolver{ioc};
                asio::connect(
                    sock.next_layer(),
                    resolver.resolve(host, std::to_string(port))
                );
                sock.handshake(ssl::stream_base::client);

                auto users = UserDB::instance().getAllUsers();
                for (auto& u : users) {
                    std::ostringstream os;
                    os << u.username << " " << u.hash << " " << u.timestamp;
                    auto msg = os.str();
                    uint32_t len = htonl(static_cast<uint32_t>(msg.size()));
                    asio::write(sock, asio::buffer(&len, 4));
                    asio::write(sock, asio::buffer(msg));
                }
            } catch (const std::exception& e) {
                std::cerr << "SyncService: unable to connect to "
                          << host << ":" << port << " — " << e.what() << "\n";
            }
        }).detach();
    }
}
