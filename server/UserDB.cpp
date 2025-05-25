#include "UserDB.h"
#include "Auth.h"
#include <chrono>
#include <iostream>

UserDB& UserDB::instance() {
    static UserDB inst;
    return inst;
}

void UserDB::addUser(const std::string& user, const std::string& hash) {
    std::lock_guard<std::mutex> lk(mtx);
    uint64_t ts = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    db[user] = {user, hash, ts};
    std::cout<<"[UserDB] Added user: "<<user<<"\n";
}

bool UserDB::verifyUser(const std::string& user, const std::string& pwd) {
    std::lock_guard<std::mutex> lk(mtx);
    auto it = db.find(user);
    if(it==db.end()) return false;
    return Auth::verifyPassword(pwd, it->second.hash);
}

std::vector<UserRecord> UserDB::getAllUsers() {
    std::lock_guard<std::mutex> lk(mtx);
    std::vector<UserRecord> v;
    for(auto &p: db) v.push_back(p.second);
    return v;
}

void UserDB::mergeUser(const UserRecord& rec) {
    std::lock_guard<std::mutex> lk(mtx);
    auto it = db.find(rec.username);
    if(it==db.end() || it->second.timestamp < rec.timestamp) {
        db[rec.username] = rec;
        std::cout<<"[UserDB] Merged user: "<<rec.username<<"\n";
    }
}