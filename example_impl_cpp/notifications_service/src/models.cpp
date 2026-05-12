#include "models.hpp"

#include <random>
#include <sstream>
#include <iomanip>

namespace notifications_service {

std::unordered_map<std::string, Notification> notifications_db;
std::mutex db_mutex;

std::string generate_notification_id() {
    // Generate UUID-like string (32 hex characters)
    std::random_device rd;
    std::uniform_int_distribution<uint16_t> dist(0, 255);
    
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        uint16_t byte = dist(rd);
        ss << std::hex << std::setw(2) << std::setfill('0') << byte;
    }
    
    std::string uuid = ss.str();
    // Format as 8-4-4-4-12 (like standard UUID)
    if (uuid.size() == 32) {
        uuid = uuid.substr(0, 8) + "-" + uuid.substr(8, 4) + "-" +
               uuid.substr(12, 4) + "-" + uuid.substr(16, 4) + "-" +
               uuid.substr(20, 12);
    }
    return uuid;
}

} // namespace notifications_service