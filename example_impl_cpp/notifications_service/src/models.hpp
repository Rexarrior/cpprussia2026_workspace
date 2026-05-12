#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <mutex>
#include <random>

namespace notifications_service {

struct Notification {
    std::string notification_id;
    std::variant<int, std::string> channel_id;  // int for regular channels, string for direct
    int message_id;
    std::string from_user_login;
    std::string to_user_login;
    bool read = false;
};

// In-memory storage with thread safety
extern std::unordered_map<std::string, Notification> notifications_db;
extern std::mutex db_mutex;

std::string generate_notification_id();

} // namespace notifications_service