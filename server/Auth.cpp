#include "Auth.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <mutex>
#include <unordered_set>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

static std::vector<unsigned char> fromHex(const std::string& hex);
static std::unordered_set<std::string> validCodes;
static std::mutex codeMutex;
static const std::string regCodeFile = "regcodes.txt";

// helper: read all lines into validCodes
static void loadCodes() {
    std::ifstream in(regCodeFile);
    std::string line;
    while(std::getline(in, line)) {
        if(line.empty() || line[0]=='#') continue;
        validCodes.insert(line);
    }
}

// append one code to the file
static void persistCode(const std::string &code) {
    std::ofstream out(regCodeFile, std::ios::app);
    out << code << "\n";
}

// Helper: hex-encode a buffer
static std::string toHex(const unsigned char* buf, size_t len) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for(size_t i=0;i<len;i++) ss<<std::setw(2)<<(int)buf[i];
    return ss.str();
}

// Helper: hex-decode into a vector
static std::vector<unsigned char> fromHex(const std::string& hex);

std::string Auth::hashPassword(const std::string& pwd) {
    // salt 16 bytes
    unsigned char salt[16];
    if(RAND_bytes(salt, sizeof(salt))!=1)
        throw std::runtime_error("RAND_bytes failed");
    // derive key: PBKDF2-HMAC-SHA256, 100k iters, 32-byte key
    unsigned char key[32];
    if(!PKCS5_PBKDF2_HMAC(
        pwd.c_str(), pwd.size(),
        salt, sizeof(salt),
        100000, EVP_sha256(),
        sizeof(key), key))
    {
        throw std::runtime_error("PBKDF2 failed");
    }
    // store as hex_salt$hex_key
    return toHex(salt,sizeof(salt)) + "$" + toHex(key,sizeof(key));
}

bool Auth::verifyPassword(const std::string& pwd, const std::string& stored) {
    auto pos = stored.find('$');
    if(pos==std::string::npos) return false;
    auto saltHex = stored.substr(0,pos);
    auto keyHex  = stored.substr(pos+1);
    auto salt = fromHex(saltHex);
    auto origKey = fromHex(keyHex);
    // derive key with same params
    std::vector<unsigned char> key(origKey.size());
    if(!PKCS5_PBKDF2_HMAC(
        pwd.c_str(), pwd.size(),
        salt.data(), salt.size(),
        100000, EVP_sha256(),
        key.size(), key.data()))
    {
        return false;
    }
    return CRYPTO_memcmp(key.data(), origKey.data(), key.size())==0;
}

bool Auth::validateRegCode(const std::string& code) {
    std::lock_guard<std::mutex> lg(codeMutex);
    return validCodes.erase(code)>0;
}

void Auth::invalidateRegCode(const std::string& /*code*/) {
    // already removed in validateRegCode
}

void Auth::addRegCode(const std::string& code) {
    {
        std::lock_guard<std::mutex> lg(codeMutex);
        validCodes.insert(code);
    }
    persistCode(code);
}

void Auth::loadCodes() {
    std::lock_guard<std::mutex> lg(codeMutex);
    validCodes.clear();
    std::ifstream in(regCodeFile);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        validCodes.insert(line);
    }
}

std::vector<std::string> Auth::listCodes() {
    std::lock_guard<std::mutex> lg(codeMutex);
    return { validCodes.begin(), validCodes.end() };
}

static std::vector<unsigned char> fromHex(const std::string& hex) {
    std::vector<unsigned char> out;
    out.reserve(hex.length() / 2);

    for (size_t i = 0; i < hex.length(); i += 2) {
        unsigned int byte = 0;
        // convert each two-char chunk of hex into a byte
        std::istringstream(hex.substr(i, 2)) >> std::hex >> byte;
        out.push_back(static_cast<unsigned char>(byte));
    }

    return out;
}