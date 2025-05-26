#include "AdminConsole.h"
#include "Auth.h"
#include "UserDB.h"
#include "ConfigUtil.h"
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

static std::string masterKey;

void AdminConsole::loadMasterKey(const std::string& path) {
    std::ifstream in(path);
    if(!in) throw std::runtime_error("Cannot open admin.key");
    std::getline(in, masterKey);
}



void AdminConsole::authenticate() {
    std::cout << "Admin console requires master key." << std::endl;
    std::cout << "Enter master key: ";
    std::string input;
    std::getline(std::cin, input);
    if(input != masterKey) {
        std::cerr << "Invalid master key. Exiting." << std::endl;
        exit(1);
    }
    std::cout << "Admin authenticated." << std::endl;
}

// helper: generate alphanumeric code
static std::string genRandom(int len=16){
    static const char* chars =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<> dist(0,61);
    std::string s;
    for(int i=0;i<len;i++)
      s.push_back(chars[dist(rng)]);
    return s;
}

void AdminConsole::run() {
    std::cout<<"Admin console. type 'help' for commands.\n";
    std::string line;
    while(true) {
        std::cout<<"> ";
        if(!std::getline(std::cin, line)) break;
        std::istringstream is(line);
        std::string cmd; is>>cmd;
        if(cmd=="exit") break;
        else if(cmd=="help") {
            std::cout
              <<" gen_code [len]       — generate & persist a new reg code\n"
              <<" list_codes           — show valid registration codes\n"
              <<" list_users           — show users + status\n"
              <<" disable_user <u>     — disable login for user u\n"
              <<" enable_user <u>      — re-enable user u\n"
              <<" add_server <h:p>     — add peer to echatserv.config\n"
              <<" rm_server <h:p>      — remove peer\n"
              <<" list_servers         — list current peers\n"
              <<" set_ssl <cert> <key> — update SSL paths\n"
              <<" exit                 — quit console\n";
        }
        else if(cmd=="gen_code") {
            int len=16; if(is>>len) {}
            auto code = genRandom(len);
            Auth::addRegCode(code);
            std::cout<<"New code: "<<code<<"\n";
        }
        else if (cmd == "list_codes") {
            auto codes = Auth::listCodes();
            std::cout << "Codes:\n";
            for (auto &c : codes)
            std::cout << "  " << c << "\n";
        }
        else if(cmd=="list_users") {
            auto users = UserDB::instance().listUsers();
            std::cout<<"Users:\n";
            for(auto &u: users)
              std::cout<<"  "<<u.username
                       <<(u.disabled?" [disabled]":"")<<"\n";
        }
        else if(cmd=="disable_user") {
            std::string u; if(is>>u && UserDB::instance().disableUser(u))
                std::cout<<"Disabled "<<u<<"\n";
            else std::cout<<"Error disabling user\n";
        }
        else if(cmd=="enable_user") {
            std::string u; if(is>>u && UserDB::instance().enableUser(u))
                std::cout<<"Enabled "<<u<<"\n";
            else std::cout<<"Error enabling user\n";
        }
        else if(cmd=="add_server") {
            std::string peer; 
            if(is>>peer && ConfigUtil::addPeer("echatserv.config",peer))
                std::cout<<"Added server "<<peer<<"\n";
            else std::cout<<"Error adding server\n";
        }
        else if(cmd=="rm_server") {
            std::string peer; 
            if(is>>peer && ConfigUtil::removePeer("echatserv.config",peer))
                std::cout<<"Removed server "<<peer<<"\n";
            else std::cout<<"Error removing server\n";
        }
        else if(cmd=="list_servers") {
            auto peers = ConfigUtil::loadPeerList("echatserv.config");
            std::cout<<"Peers:\n";
            for(auto &p: peers) std::cout<<"  "<<p<<"\n";
        }
        else if(cmd=="set_ssl") {
            std::string cert,key;
            if(is>>std::quoted(cert)>>std::quoted(key)
               && ConfigUtil::setSslPaths("echatserv.config",cert,key))
                std::cout<<"SSL paths updated\n";
            else std::cout<<"Usage: set_ssl \"cert.pem\" \"key.pem\"\n";
        }
        else {
            std::cout<<"Unknown command: "<<cmd<<"\n";
        }
    }
}