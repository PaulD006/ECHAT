#pragma once
#include <string>

namespace Auth {
    std::string hashPassword(const std::string& pwd);
    bool verifyPassword(const std::string& pwd, const std::string& stored);
    bool validateRegCode(const std::string& code);
    void invalidateRegCode(const std::string& code);
    void addRegCode(const std::string& code);
}