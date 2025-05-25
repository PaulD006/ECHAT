#include "Auth.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <mutex>
#include <unordered_set>
#include <stdexcept>
#include <sstream>
#include <iomanip>

static std::unordered_set<std::string> validCodes;
static std::mutex codeMutex;

// Helper: hex-encode a buffer
static std::string toHex(const unsigned char* buf, size_t len) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for(size_t i=0;i<len;i++) ss<<std::setw(2)<<(int)buf[i];
    return ss.str();
}

// Helper: hex-decode into a vector
static std::vector<unsigned char> fromHex(const std::string& hex) {
    std::vector<unsigned char> out(hex.size()/2);
    for(size_t i=0;i<out.size();i++){
        unsigned int byte;
        std::istringstream(hex.substr(2*i,2))>>std::hex>>byte;
        out[i]=static_cast<unsigned char>(byte);
    }
    return out;
}

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
    std::lock_guard<std::mutex> lg(codeMutex);
    validCodes.insert(code);
}