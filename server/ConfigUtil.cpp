#include "ConfigUtil.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;

static constexpr const char* SSL_HEADER = "[ssl]";

vector<string> ConfigUtil::loadPeerList(const string& path) {
    ifstream in(path);
    vector<string> peers;
    string line;
    while(getline(in, line)) {
        if(line == SSL_HEADER) break;
        if(line.empty() || line[0]=='#') continue;
        peers.push_back(line);
    }
    return peers;
}

bool ConfigUtil::savePeerList(const string& path, const vector<string>& peers) {
    ifstream in(path);
    if(!in) return false;
    vector<string> pre, post;
    string line;
    // gather lines until [ssl]
    while(getline(in, line)) {
        if(line == SSL_HEADER) break;
        pre.push_back(line);
    }
    // now gather rest (including [ssl] and beyond)
    post.push_back(SSL_HEADER);
    while(getline(in, line)) post.push_back(line);
    in.close();

    ofstream out(path);
    if(!out) return false;
    // write preamble up to peers header comment (we assume first lines are comments)
    for(auto &l: pre) {
        if(l.empty() || l[0]=='#') out<<l<<"\n";
    }
    // write new peers
    for(auto &p: peers) out<<p<<"\n";
    out<<"\n";
    // write ssl block
    for(auto &l: post) out<<l<<"\n";
    return true;
}

bool ConfigUtil::addPeer(const string& path, const string& peer) {
    auto peers = loadPeerList(path);
    if(find(peers.begin(), peers.end(), peer)!=peers.end()) return false;
    peers.push_back(peer);
    return savePeerList(path, peers);
}

bool ConfigUtil::removePeer(const string& path, const string& peer) {
    auto peers = loadPeerList(path);
    auto it = remove(peers.begin(), peers.end(), peer);
    if(it==peers.end()) return false;
    peers.erase(it, peers.end());
    return savePeerList(path, peers);
}

bool ConfigUtil::setSslPaths(const string& path,
                             const string& certChain,
                             const string& privKey) {
    ifstream in(path);
    if(!in) return false;
    vector<string> lines;
    string line;
    bool inSsl = false;
    while(getline(in,line)) {
        if(line == SSL_HEADER) {
            inSsl = true;
            lines.push_back(line);
            continue;
        }
        if(!inSsl) {
            lines.push_back(line);
        } else {
            // skip old ssl lines
            if(line.rfind("certificate_chain_file",0)==0) {
                lines.push_back("certificate_chain_file = \""+certChain+"\"");
            }
            else if(line.rfind("private_key_file",0)==0) {
                lines.push_back("private_key_file      = \""+privKey+"\"");
            } else {
                lines.push_back(line);
            }
        }
    }
    in.close();
    ofstream out(path);
    if(!out) return false;
    for(auto &l: lines) out<<l<<"\n";
    return true;
}