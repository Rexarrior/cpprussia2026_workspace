#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <stdexcept>

namespace files_service {

namespace json = userver::formats::json;

V1FileNewRequest ParseFileNewRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1FileNewRequest req{
        .login = json["login"].As<std::string>(),
        .filename = json["filename"].As<std::string>(),
        .content = json["content"].As<std::string>()
    };
    
    // Validation
    if (req.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    if (req.filename.size() < 3 || req.filename.size() > 256) {
        throw std::runtime_error("filename must be between 3 and 256 characters");
    }
    // content can be empty?
    return req;
}

V1FileByUriRequest ParseFileByUriRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1FileByUriRequest req{
        .current_user = V1CurrentUser{
            .token = json["current_user"]["token"].As<std::string>(),
            .login = json["current_user"]["login"].As<std::string>(),
            .name = json["current_user"]["name"].As<std::string>()
        },
        .uri = json["uri"].As<std::string>()
    };
    
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be exactly 128 characters");
    }
    if (req.current_user.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    if (req.uri.size() < 3) {
        throw std::runtime_error("uri must be at least 3 characters");
    }
    return req;
}

std::string ToJson(const V1FileNewResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder current_user;
    current_user["token"] = response.current_user.token;
    current_user["login"] = response.current_user.login;
    current_user["name"] = response.current_user.name;
    builder["current_user"] = std::move(current_user);
    builder["uri"] = response.uri;
    json::ValueBuilder file;
    file["login"] = response.file.login;
    file["filename"] = response.file.filename;
    file["content"] = response.file.content;
    builder["file"] = std::move(file);
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1FileByUriResponse& response) {
    json::ValueBuilder builder;
    builder["login"] = response.login;
    builder["filename"] = response.filename;
    builder["content"] = response.content;
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1Error& error) {
    json::ValueBuilder builder;
    builder["code"] = error.code;
    builder["message"] = error.message;
    if (error.details.has_value()) {
        json::ValueBuilder details_obj;
        for (const auto& [key, value] : *error.details) {
            details_obj[key] = value;
        }
        builder["details"] = std::move(details_obj);
    }
    return json::ToString(builder.ExtractValue());
}

} // namespace files_service