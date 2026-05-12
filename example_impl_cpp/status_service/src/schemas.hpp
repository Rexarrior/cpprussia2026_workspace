#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace status_service {

// Common types
struct V1CurrentUser {
    std::string token;
    std::string login;
    std::string name;
};

struct V1UserStatusUpdateRequest {
    V1CurrentUser current_user;
    std::unordered_map<std::string, std::string> status; // simple dict
};

struct V1UserStatusUpdateResponse {
    bool success;
    std::string updated_at;
    std::optional<std::string> expires_at;
};

struct V1UserStatusByLoginRequest {
    V1CurrentUser current_user;
    std::string login;
};

struct V1UserStatusByLoginResponse {
    std::unordered_map<std::string, std::string> status;
};

struct V1Error {
    std::string code;
    std::string message;
    std::optional<std::unordered_map<std::string, std::string>> details = std::nullopt;
};

// Helper functions to parse JSON requests and generate JSON responses
V1UserStatusUpdateRequest ParseUpdateRequest(const std::string& json);
V1UserStatusByLoginRequest ParseByLoginRequest(const std::string& json);

std::string ToJson(const V1UserStatusUpdateResponse& response);
std::string ToJson(const V1UserStatusByLoginResponse& response);
std::string ToJson(const V1Error& error);

} // namespace status_service