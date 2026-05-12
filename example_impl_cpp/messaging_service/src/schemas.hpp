#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>

namespace messaging_service {

// Common types
struct V1CurrentUser {
    std::string token;
    std::string login;
    std::string name;
};

struct V1PublicUser {
    std::string login;
    std::string name;
};

struct V1ChannelMessage {
    V1CurrentUser current_user;
    int id;
    std::string timestamp;
    std::string message;
};

struct V1Error {
    std::string code;
    std::string message;
    std::optional<std::unordered_map<std::string, std::string>> details = std::nullopt;
};

// Request schemas
struct V1ChannelMessageByTimestampRequest {
    std::variant<int, std::string> channel_id;
    std::string from;
    std::optional<std::string> to;
    int limit = 100;
};

struct V1ChannelMessageNewRequest {
    V1CurrentUser current_user;
    std::variant<int, std::string> channel_id;
    std::string message;
};

struct V1DirectGetOrCreateRequest {
    V1CurrentUser current_user;
    std::string other_user_login;
};

struct V1DirectListRequest {
    V1CurrentUser current_user;
};

// Response schemas
struct V1ChannelMessageByTimestampResponse {
    std::vector<V1ChannelMessage> messages;
    std::optional<std::string> next_cursor;
    bool has_more = false;
};

struct V1ChannelMessageNewResponse {
    int message_id;
};

struct V1DirectGetOrCreateResponse {
    std::string channel_id;
    V1PublicUser other_user;
};

struct V1DirectChatInfo {
    std::string channel_id;
    V1PublicUser other_user;
    std::optional<V1ChannelMessage> last_message;
};

struct V1DirectListResponse {
    std::vector<V1DirectChatInfo> direct_chats;
};

// Helper functions to parse JSON requests and generate JSON responses
V1ChannelMessageByTimestampRequest ParseChannelMessageByTimestampRequest(const std::string& json);
V1ChannelMessageNewRequest ParseChannelMessageNewRequest(const std::string& json);
V1DirectGetOrCreateRequest ParseDirectGetOrCreateRequest(const std::string& json);
V1DirectListRequest ParseDirectListRequest(const std::string& json);

std::string ToJson(const V1ChannelMessageByTimestampResponse& response);
std::string ToJson(const V1ChannelMessageNewResponse& response);
std::string ToJson(const V1DirectGetOrCreateResponse& response);
std::string ToJson(const V1DirectListResponse& response);
std::string ToJson(const V1Error& error);

} // namespace messaging_service