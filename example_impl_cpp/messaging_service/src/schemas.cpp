#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <fmt/format.h>

#include <algorithm>

namespace messaging_service {

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

V1ChannelMessageByTimestampRequest ParseChannelMessageByTimestampRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1ChannelMessageByTimestampRequest req;
    req.channel_id = ParseChannelId(json["channel_id"]);
    req.from = json["from"].As<std::string>();
    if (json.HasMember("to")) {
        req.to = json["to"].As<std::string>();
    }
    if (json.HasMember("limit")) {
        req.limit = json["limit"].As<int>();
    }
    // Validation
    if (req.from.empty()) {
        throw std::runtime_error("from must not be empty");
    }
    if (req.limit < 1 || req.limit > 1000) {
        throw std::runtime_error("limit must be between 1 and 1000");
    }
    return req;
}

V1ChannelMessageNewRequest ParseChannelMessageNewRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1ChannelMessageNewRequest req;
    req.current_user.token = json["current_user"]["token"].As<std::string>();
    req.current_user.login = json["current_user"]["login"].As<std::string>();
    req.current_user.name = json["current_user"]["name"].As<std::string>();
    req.channel_id = ParseChannelId(json["channel_id"]);
    req.message = json["message"].As<std::string>();
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be 128 characters");
    }
    if (req.message.empty()) {
        throw std::runtime_error("message must not be empty");
    }
    return req;
}

V1DirectGetOrCreateRequest ParseDirectGetOrCreateRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1DirectGetOrCreateRequest req;
    req.current_user.token = json["current_user"]["token"].As<std::string>();
    req.current_user.login = json["current_user"]["login"].As<std::string>();
    req.current_user.name = json["current_user"]["name"].As<std::string>();
    req.other_user_login = json["other_user_login"].As<std::string>();
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be 128 characters");
    }
    if (req.other_user_login.empty()) {
        throw std::runtime_error("other_user_login must not be empty");
    }
    return req;
}

V1DirectListRequest ParseDirectListRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1DirectListRequest req;
    req.current_user.token = json["current_user"]["token"].As<std::string>();
    req.current_user.login = json["current_user"]["login"].As<std::string>();
    req.current_user.name = json["current_user"]["name"].As<std::string>();
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be 128 characters");
    }
    return req;
}

// Helper to serialize variant channel_id
json::ValueBuilder SerializeChannelId(const std::variant<int, std::string>& channel_id) {
    if (std::holds_alternative<int>(channel_id)) {
        return json::ValueBuilder(std::get<int>(channel_id));
    } else {
        return json::ValueBuilder(std::get<std::string>(channel_id));
    }
}

std::string ToJson(const V1ChannelMessageByTimestampResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder messages_array(json::Type::kArray);
    for (const auto& msg : response.messages) {
        json::ValueBuilder msg_obj;
        json::ValueBuilder current_user;
        current_user["token"] = msg.current_user.token;
        current_user["login"] = msg.current_user.login;
        current_user["name"] = msg.current_user.name;
        msg_obj["current_user"] = std::move(current_user);
        msg_obj["id"] = msg.id;
        msg_obj["timestamp"] = msg.timestamp;
        msg_obj["message"] = msg.message;
        messages_array.PushBack(std::move(msg_obj));
    }
    builder["messages"] = std::move(messages_array);
    if (response.next_cursor.has_value()) {
        builder["next_cursor"] = *response.next_cursor;
    } else {
        builder["next_cursor"] = nullptr;
    }
    builder["has_more"] = response.has_more;
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1ChannelMessageNewResponse& response) {
    json::ValueBuilder builder;
    builder["message_id"] = response.message_id;
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1DirectGetOrCreateResponse& response) {
    json::ValueBuilder builder;
    builder["channel_id"] = response.channel_id;
    json::ValueBuilder other_user;
    other_user["login"] = response.other_user.login;
    other_user["name"] = response.other_user.name;
    builder["other_user"] = std::move(other_user);
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1DirectListResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder chats_array(json::Type::kArray);
    for (const auto& chat : response.direct_chats) {
        json::ValueBuilder chat_obj;
        chat_obj["channel_id"] = chat.channel_id;
        json::ValueBuilder other_user;
        other_user["login"] = chat.other_user.login;
        other_user["name"] = chat.other_user.name;
        chat_obj["other_user"] = std::move(other_user);
        if (chat.last_message.has_value()) {
            json::ValueBuilder last_msg;
            const auto& msg = *chat.last_message;
            json::ValueBuilder current_user;
            current_user["token"] = msg.current_user.token;
            current_user["login"] = msg.current_user.login;
            current_user["name"] = msg.current_user.name;
            last_msg["current_user"] = std::move(current_user);
            last_msg["id"] = msg.id;
            last_msg["timestamp"] = msg.timestamp;
            last_msg["message"] = msg.message;
            chat_obj["last_message"] = std::move(last_msg);
        } else {
            chat_obj["last_message"] = nullptr;
        }
        chats_array.PushBack(std::move(chat_obj));
    }
    builder["direct_chats"] = std::move(chats_array);
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

} // namespace messaging_service