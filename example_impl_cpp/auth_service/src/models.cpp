#include "models.hpp"

#include <openssl/sha.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <mutex>

namespace auth_service {

std::unordered_map<std::string, User> users_db;
std::unordered_map<std::string, std::string> tokens_db; // token -> login
std::mutex db_mutex;

std::string hash_password(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool verify_password(const std::string& password, const std::string& password_hash) {
    return hash_password(password) == password_hash;
}

std::string generate_token() {
    // Generate cryptographically secure 128 hex characters (512 bits)
    std::random_device rd;
    std::uniform_int_distribution<uint16_t> dist(0, 255);
    
    std::stringstream ss;
    for (int i = 0; i < 64; ++i) {
        uint16_t byte = dist(rd);
        ss << std::hex << std::setw(2) << std::setfill('0') << byte;
    }
    
    std::string token = ss.str();
    // Ensure exactly 128 characters
    if (token.size() != 128) {
        // Should never happen, but fallback
        token = std::string(128, '0');
    }
    return token;
}

} // namespace auth_service