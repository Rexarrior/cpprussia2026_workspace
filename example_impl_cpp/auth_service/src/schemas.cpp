#include "schemas.hpp"

#include <userver/formats/json.hpp>


namespace auth_service {

namespace json = userver::formats::json;

V1UserRegistrationRequest ParseRegistrationRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1UserRegistrationRequest req{
        .login = json["login"].As<std::string>(),
        .name = json["name"].As<std::string>(),
        .email = json["email"].As<std::string>(),
        .phone = json["phone"].As<std::string>(),
        .password = json["password"].As<std::string>()
    };
    
    // Validation
    if (req.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    if (req.name.size() < 1) {
        throw std::runtime_error("name must be at least 1 character");
    }
    if (req.email.size() < 3) {
        throw std::runtime_error("email must be at least 3 characters");
    }
    if (req.phone.size() < 3) {
        throw std::runtime_error("phone must be at least 3 characters");
    }
    if (req.password.size() < 6) {
        throw std::runtime_error("password must be at least 6 characters");
    }
    
    return req;
}

V1UserAuthorizationRequest ParseAuthorizationRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1UserAuthorizationRequest req{
        .login = json["login"].As<std::string>(),
        .password = json["password"].As<std::string>()
    };
    
    // Validation
    if (req.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    if (req.password.size() < 6) {
        throw std::runtime_error("password must be at least 6 characters");
    }
    
    return req;
}

V1UserSearchRequest ParseSearchRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1UserSearchRequest req{
        .query = json["query"].As<std::string>(),
        .limit = json["limit"].As<int>(10)
    };
    
    // Validation
    if (req.query.size() < 1) {
        throw std::runtime_error("query must be at least 1 character");
    }
    if (req.limit < 1 || req.limit > 50) {
        throw std::runtime_error("limit must be between 1 and 50");
    }
    
    return req;
}

std::string ToJson(const V1UserAuthorizationResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder current_user;
    current_user["token"] = response.current_user.token;
    current_user["login"] = response.current_user.login;
    current_user["name"] = response.current_user.name;
    builder["current_user"] = std::move(current_user);
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1UserSearchResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder users_array(json::Type::kArray);
    for (const auto& user : response.users) {
        json::ValueBuilder user_obj;
        user_obj["login"] = user.login;
        user_obj["name"] = user.name;
        users_array.PushBack(std::move(user_obj));
    }
    builder["users"] = std::move(users_array);
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

} // namespace auth_service