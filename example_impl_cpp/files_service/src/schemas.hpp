#pragma once

#include <string>
#include <optional>
#include <unordered_map>

namespace files_service {

// Request schemas
struct V1Login {
    std::string login;
};

struct V1CurrentUser {
    std::string token;
    std::string login;
    std::string name;
};

struct V1File {
    std::string login;
    std::string filename;
    std::string content;
};

struct V1FileNewRequest {
    std::string login;
    std::string filename;
    std::string content;
};

struct V1FileNewResponse {
    V1CurrentUser current_user;
    std::string uri;
    V1File file;
};

struct V1FileByUriRequest {
    V1CurrentUser current_user;
    std::string uri;
};

struct V1FileByUriResponse {
    std::string login;
    std::string filename;
    std::string content;
};

struct V1Error {
    std::string code;
    std::string message;
    std::optional<std::unordered_map<std::string, std::string>> details = std::nullopt;
};

// Helper functions to parse JSON requests and generate JSON responses
V1FileNewRequest ParseFileNewRequest(const std::string& json);
V1FileByUriRequest ParseFileByUriRequest(const std::string& json);

std::string ToJson(const V1FileNewResponse& response);
std::string ToJson(const V1FileByUriResponse& response);
std::string ToJson(const V1Error& error);

} // namespace files_service