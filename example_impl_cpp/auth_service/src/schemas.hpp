#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace auth_service {

// Request schemas
struct V1UserRegistrationRequest {
    std::string login;
    std::string name;
    std::string email;
    std::string phone;
    std::string password;
};

struct V1UserAuthorizationRequest {
    std::string login;
    std::string password;
};

struct V1UserSearchRequest {
    std::string query;
    int limit = 10;
};

// Response schemas
struct V1PublicUser {
    std::string login;
    std::string name;
};

struct V1AuthorizedUser {
    std::string token;
    std::string login;
    std::string name;
};

struct V1UserAuthorizationResponse {
    V1AuthorizedUser current_user;
};

struct V1UserSearchResponse {
    std::vector<V1PublicUser> users;
};

struct V1Error {
    std::string code;
    std::string message;
    std::optional<std::unordered_map<std::string, std::string>> details = std::nullopt;
};

// Helper functions to parse JSON requests and generate JSON responses
V1UserRegistrationRequest ParseRegistrationRequest(const std::string& json);
V1UserAuthorizationRequest ParseAuthorizationRequest(const std::string& json);
V1UserSearchRequest ParseSearchRequest(const std::string& json);

std::string ToJson(const V1UserAuthorizationResponse& response);
std::string ToJson(const V1UserSearchResponse& response);
std::string ToJson(const V1Error& error);

} // namespace auth_service