#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <userver/formats/common/items.hpp>
#include <stdexcept>
#include <unordered_set>

namespace status_service {

namespace json = userver::formats::json;
namespace common = userver::formats::common;

V1UserStatusUpdateRequest ParseUpdateRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1UserStatusUpdateRequest req{
        .current_user = V1CurrentUser{
            .token = json["current_user"]["token"].As<std::string>(),
            .login = json["current_user"]["login"].As<std::string>(),
            .name = json["current_user"]["name"].As<std::string>()
        },
        .status = {}
    };
    // Parse status dict
    auto status_obj = json["status"];
    for (const auto& [key, value] : common::Items(status_obj)) {
        req.status[key] = value.As<std::string>();
    }
    
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be exactly 128 characters");
    }
    if (req.current_user.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    // Optional validation of status_type and visibility will be done in handler
    return req;
}

V1UserStatusByLoginRequest ParseByLoginRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1UserStatusByLoginRequest req{
        .current_user = V1CurrentUser{
            .token = json["current_user"]["token"].As<std::string>(),
            .login = json["current_user"]["login"].As<std::string>(),
            .name = json["current_user"]["name"].As<std::string>()
        },
        .login = json["login"].As<std::string>()
    };
    
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be exactly 128 characters");
    }
    if (req.current_user.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    if (req.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    return req;
}

std::string ToJson(const V1UserStatusUpdateResponse& response) {
    json::ValueBuilder builder;
    builder["success"] = response.success;
    builder["updated_at"] = response.updated_at;
    if (response.expires_at.has_value()) {
        builder["expires_at"] = *response.expires_at;
    } else {
        builder["expires_at"] = json::ValueBuilder(json::Type::kNull);
    }
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1UserStatusByLoginResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder status_obj;
    for (const auto& [key, value] : response.status) {
        status_obj[key] = value;
    }
    builder["status"] = std::move(status_obj);
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

} // namespace status_service