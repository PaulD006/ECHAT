#include "SyncService.h"
#include "UserDB.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <thread>
#include <fstream>
#include <iostream>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace asio = boost::asio;

SyncService& SyncService::instance() {
    static SyncService inst;
    return inst;
}

SyncService::SyncService() {}

void SyncService::loadPeerConfig(const std::string& path) {
    std::ifstream in(path);
    std::string line;
    while(std::getline(in, line)) {
        if(line.empty()||line[0]=='#') continue;
        auto pos = line.find(':');
        std::string host = line.substr(0,pos);
        int port = std::stoi(line.substr(pos+1));
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
        ctx.use_certificate_chain_file("cert.pem");
        ctx.use_private_key_file("key.pem", ssl::context::pem);
        tcp::acceptor acceptor{ioc, {tcp::v4(), 9001}};
        for(;;) {
            ssl::stream<tcp::socket> sock{ioc, ctx};
            acceptor.accept(sock.next_layer());
            sock.handshake(ssl::stream_base::server);
            // receive initial USER list or updates
            std::thread([s=std::move(sock)]() mutable {
                try {
                    for(;;) {
                        // simple protocol: read length, then data
                        uint32_t len;
                        asio::read(s, asio::buffer(&len,4));
                        len = ntohl(len);
                        std::vector<char> buf(len);
                        asio::read(s, asio::buffer(buf));
                        UserRecord rec;
                        // deserialize rec (e.g., CSV or binary)
                        // here assume: username\nhash\ntimestamp
                        std::istringstream is(std::string(buf.begin(),buf.end()));
                        is>>rec.username>>rec.hash>>rec.timestamp;
                        UserDB::instance().mergeUser(rec);
                    }
                } catch(...){}
            }).detach();
        }
    } catch(const std::exception& e) {
        std::cerr<<"Sync accept error: "<<e.what()<<"\n";
    }
}

void SyncService::connectLoop() {
    for(auto& p : peers) {
        std::thread([host=p.first,port=p.second]{
            asio::io_context ioc;
            ssl::context ctx(ssl::context::tlsv12_client);
            ctx.load_verify_file("cert.pem");
            ssl::stream<tcp::socket> sock(ioc, ctx);
            tcp::resolver resolver{ioc};
            asio::connect(sock.next_layer(), resolver.resolve(host, std::to_string(port)));
            sock.handshake(ssl::stream_base::client);
            // on connect, send full USER list
            auto users = UserDB::instance().getAllUsers();
            for(auto& u: users) {
                std::ostringstream os;
                os<<u.username<<" "<<u.hash<<" "<<u.timestamp;
                auto msg = os.str();
                uint32_t len = htonl((uint32_t)msg.size());
                asio::write(sock, asio::buffer(&len,4));
                asio::write(sock, asio::buffer(msg));
            }
            // keep connection alive to receive updates (not fully implemented)
        }).detach();
    }
}