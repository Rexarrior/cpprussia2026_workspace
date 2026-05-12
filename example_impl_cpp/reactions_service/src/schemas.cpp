#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <stdexcept>
#include <unordered_set>

namespace reactions_service {

namespace json = userver::formats::json;

V1LikeTriggerRequest ParseLikeTriggerRequest(const std::string& json_str) {
    auto json = json::FromString(json_str);
    V1LikeTriggerRequest req{
        .current_user = V1CurrentUser{
            .token = json["current_user"]["token"].As<std::string>(),
            .login = json["current_user"]["login"].As<std::string>(),
            .name = json["current_user"]["name"].As<std::string>()
        },
        .idempotency_token = json["idempotency_token"].As<std::string>(),
        .channel_id = json["channel_id"].As<int>(),
        .message_id = json["message_id"].As<int>(),
        .animation = json["animation"].As<std::string>()
    };
    
    // Validation
    if (req.current_user.token.size() != 128) {
        throw std::runtime_error("token must be exactly 128 characters");
    }
    if (req.current_user.login.size() < 3) {
        throw std::runtime_error("login must be at least 3 characters");
    }
    if (req.idempotency_token.size() < 16 || req.idempotency_token.size() > 256) {
        throw std::runtime_error("idempotency_token must be between 16 and 256 characters");
    }
    if (req.channel_id < 0) {
        throw std::runtime_error("channel_id must be non-negative");
    }
    if (req.message_id < 0) {
        throw std::runtime_error("message_id must be non-negative");
    }
    static const std::unordered_set<std::string> valid_animations = {
        "like", "dislike", "heart", "fire", "okay", "LOL", "smile"
    };
    if (valid_animations.find(req.animation) == valid_animations.end()) {
        throw std::runtime_error("invalid animation type");
    }
    return req;
}

std::string ToJson(const V1LikeTriggerResponse& response) {
    json::ValueBuilder builder;
    builder["action"] = response.action;
    if (response.current_user_reaction.has_value()) {
        builder["current_user_reaction"] = *response.current_user_reaction;
    } else {
        builder["current_user_reaction"] = json::ValueBuilder(json::Type::kNull);
    }
    return json::ToString(builder.ExtractValue());
}

std::string ToJson(const V1GetReactionsResponse& response) {
    json::ValueBuilder builder;
    json::ValueBuilder reactions_array(json::Type::kArray);
    for (const auto& reaction : response.reactions) {
        json::ValueBuilder reaction_obj;
        reaction_obj["animation"] = reaction.animation;
        reaction_obj["count"] = reaction.count;
        json::ValueBuilder user_logins_array(json::Type::kArray);
        for (const auto& login : reaction.user_logins) {
            user_logins_array.PushBack(login);
        }
        reaction_obj["user_logins"] = std::move(user_logins_array);
        reactions_array.PushBack(std::move(reaction_obj));
    }
    builder["reactions"] = std::move(reactions_array);
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

} // namespace reactions_service