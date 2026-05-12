#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include <mutex>
#include <atomic>

namespace messaging_service {

struct Channel {
    int id;
    std::string name;
};

struct DirectChannel {
    std::string channel_id;  // Format: dm_{user1}_{user2} where user1 < user2 alphabetically
    std::string user1;
    std::string user2;
};

struct Message {
    int id;
    std::variant<int, std::string> channel_id;  // int for regular channels, string for direct
    std::string user_login;
    std::string user_name;
    std::string message;
    std::string timestamp;  // ISO 8601
};

// In-memory storage with thread safety
extern std::unordered_map<int, Channel> channels_db;
extern std::unordered_map<std::string, DirectChannel> direct_channels_db;
extern std::unordered_map<int, Message> messages_db;
extern std::atomic<int> message_counter;
extern std::mutex db_mutex;

int generate_message_id();
std::string get_direct_channel_id(const std::string& user1, const std::string& user2);

} // namespace messaging_service