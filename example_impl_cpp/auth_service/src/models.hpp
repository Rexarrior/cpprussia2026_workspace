#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <cstdint>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace auth_service {

struct User {
    std::string login;
    std::string name;
    std::string email;
    std::string phone;
    std::string password_hash;
    std::optional<std::string> token;
};

// In-memory storage with thread safety
extern std::unordered_map<std::string, User> users_db;
extern std::unordered_map<std::string, std::string> tokens_db; // token -> login
extern std::mutex db_mutex;

std::string hash_password(const std::string& password);
bool verify_password(const std::string& password, const std::string& password_hash);
std::string generate_token();

} // namespace auth_service