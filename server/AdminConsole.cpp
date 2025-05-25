#include "AdminConsole.h"
#include <iostream>
#include <fstream>
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