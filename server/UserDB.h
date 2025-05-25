#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

struct UserRecord {
    std::string username;
    std::string hash;
    uint64_t timestamp;
};

class UserDB {
public:
    static UserDB& instance();
    void addUser(const std::string& user, const std::string& hash);
    bool verifyUser(const std::string& user, const std::string& pwd);
    std::vector<UserRecord> getAllUsers();
    void mergeUser(const UserRecord& rec);
private:
    std::mutex mtx;
    std::unordered_map<std::string, UserRecord> db;
    UserDB() = default;
};