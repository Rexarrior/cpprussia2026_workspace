#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>

namespace notifications_service {

// Common types (shared with other services)
struct V1CurrentUser {
    std::string token;
    std::string login;
    std::string name;
};

struct V1NotificationStatus {
    int message_id;
    bool read = false;
};

struct V1Error {
    std::string code;
    std::string message;
    std::optional<std::unordered_map<std::string, std::string>> details = std::nullopt;
};

// Request schemas
struct V1ChannelNotificationListRequest {
    V1CurrentUser current_user;
    std::variant<int, std::string> channel_id;
};

struct V1ChannelNotificationNewRequest {
    V1CurrentUser current_user;
    std::variant<int, std::string> channel_id;
    int message_id;
    std::string other_user_login;
};

// Response schemas
struct V1ChannelNotificationListResponse {
    std::vector<V1NotificationStatus> notifications;
};

struct V1ChannelNotificationNewResponse {
    std::string notification_id;
};

// Helper functions to parse JSON requests and generate JSON responses
V1ChannelNotificationListRequest ParseListRequest(const std::string& json);
V1ChannelNotificationNewRequest ParseNewRequest(const std::string& json);

std::string ToJson(const V1ChannelNotificationListResponse& response);
std::string ToJson(const V1ChannelNotificationNewResponse& response);
std::string ToJson(const V1Error& error);

} // namespace notifications_service