#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <algorithm>

namespace notifications_service {

namespace json = userver::formats::json;

namespace {

std::variant<int, std::string> ParseChannelId(const json::Value& value) {
    if (value.IsInt()) {
        return value.As<int>();
    } else if (value.IsString()) {
        return value.As<std::string>();
    } else {
        throw std::runtime_error("channel_id must be integer or string");
    }
}

} // anonymous namespace

V1ChannelNotificationListRequest ParseListRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1ChannelNotificationListRequest req;
    req.current_user.token = json["current_user"]["token"].As<std::string>();
    req.current_user.login = json["current_user"]["login"].As<std::string>();
    req.current_user.name = json["current_user"]["name"].As<std::string>();
    req.channel_id = ParseChannelId(json["channel_id"]);
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be 128 characters");
    }
    return req;
}

V1ChannelNotificationNewRequest ParseNewRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1ChannelNotificationNewRequest req;
    req.current_user.token = json["current_user"]["token"].As<std::string>();
    req.current_user.login = json["current_user"]["login"].As<std::string>();
    req.current_user.name = json["current_user"]["name"].As<std::string>();
    req.channel_id = ParseChannelId(json["channel_id"]);
    req.message_id = json["message_id"].As<int>();
    req.other_user_login = json["other_user_login"].As<std::string>();
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be 128 characters");
    }
    if (req.other_user_login.size() < 3) {
        throw std::runtime_error("other_user_login must be at least 3 characters");
    }
    return req;
}

std::string ToJson(const V1ChannelNotificationListResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder notifications_array(json::Type::kArray);
    for (const auto& notification : response.notifications) {
        json::ValueBuilder notif_obj;
        notif_obj["message_id"] = notification.message_id;
        notif_obj["read"] = notification.read;
        notifications_array.PushBack(std::move(notif_obj));
    }
    builder["notifications"] = std::move(notifications_array);
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1ChannelNotificationNewResponse& response) {
    json::ValueBuilder builder;
    builder["notification_id"] = response.notification_id;
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

} // namespace notifications_service