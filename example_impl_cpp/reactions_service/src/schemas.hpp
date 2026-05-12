#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace reactions_service {

// Common types
struct V1CurrentUser {
    std::string token;
    std::string login;
    std::string name;
};

struct V1LikeTriggerRequest {
    V1CurrentUser current_user;
    std::string idempotency_token;
    int channel_id;
    int message_id;
    std::string animation;
};

struct V1LikeTriggerResponse {
    std::string action; // "added" or "removed"
    std::optional<std::string> current_user_reaction;
};

struct V1ReactionInfo {
    std::string animation;
    int count;
    std::vector<std::string> user_logins;
};

struct V1GetReactionsResponse {
    std::vector<V1ReactionInfo> reactions;
};

struct V1Error {
    std::string code;
    std::string message;
    std::optional<std::unordered_map<std::string, std::string>> details = std::nullopt;
};

// Helper functions to parse JSON requests and generate JSON responses
V1LikeTriggerRequest ParseLikeTriggerRequest(const std::string& json);
std::string ToJson(const V1LikeTriggerResponse& response);
std::string ToJson(const V1GetReactionsResponse& response);
std::string ToJson(const V1Error& error);

} // namespace reactions_service