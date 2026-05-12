#include "models.hpp"

namespace messaging_service {

std::unordered_map<int, Channel> channels_db = {
    {1, Channel{1, "general"}},
    {2, Channel{2, "random"}}
};
std::unordered_map<std::string, DirectChannel> direct_channels_db;
std::unordered_map<int, Message> messages_db;
std::atomic<int> message_counter{0};
std::mutex db_mutex;

int generate_message_id() {
    return ++message_counter;
}

std::string get_direct_channel_id(const std::string& user1, const std::string& user2) {
    if (user1 < user2) {
        return "dm_" + user1 + "_" + user2;
    } else {
        return "dm_" + user2 + "_" + user1;
    }
}

} // namespace messaging_service