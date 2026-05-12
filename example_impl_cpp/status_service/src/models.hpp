#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <chrono>

namespace status_service {

struct UserStatus {
    std::string login;
    std::string status_type;
    std::string status_message;
    std::string visibility;
    std::chrono::system_clock::time_point updated_at;
    std::optional<std::chrono::system_clock::time_point> expires_at;
};

constexpr int DEFAULT_TTL_SECONDS = 3600;

class StatusStorage {
public:
    static StatusStorage& GetInstance() {
        static StatusStorage instance;
        return instance;
    }

    // Store or update user status
    void SetStatus(const UserStatus& status) {
        std::lock_guard<std::mutex> lock(mutex_);
        statuses_db_[status.login] = status;
    }

    // Retrieve user status, returns nullopt if not found or expired
    std::optional<UserStatus> GetStatus(const std::string& login) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = statuses_db_.find(login);
        if (it == statuses_db_.end()) {
            return std::nullopt;
        }
        const UserStatus& status = it->second;
        // Check expiration
        if (status.expires_at.has_value() && *status.expires_at < std::chrono::system_clock::now()) {
            // Expired, remove entry
            statuses_db_.erase(it);
            return std::nullopt;
        }
        return status;
    }

    // Delete status
    bool DeleteStatus(const std::string& login) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = statuses_db_.find(login);
        if (it == statuses_db_.end()) {
            return false;
        }
        statuses_db_.erase(it);
        return true;
    }

private:
    StatusStorage() = default;
    std::unordered_map<std::string, UserStatus> statuses_db_;
    std::mutex mutex_;
};

} // namespace status_service