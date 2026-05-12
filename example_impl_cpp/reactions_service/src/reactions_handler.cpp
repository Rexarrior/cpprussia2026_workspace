#include "reactions_handler.hpp"
#include "schemas.hpp"
#include "models.hpp"

#include <userver/formats/json.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/http/http_response.hpp>
#include <fmt/format.h>
#include <stdexcept>
#include <regex>

namespace reactions_service {

ReactionsHandler::ReactionsHandler(const components::ComponentConfig& config,
                                   const components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
}

std::string ReactionsHandler::HandleRequest(server::http::HttpRequest& request,
                                            server::request::RequestContext&) const {
    const auto& path = request.GetRequestPath();
    [[maybe_unused]] const auto& method = request.GetMethod();

    request.GetHttpResponse().SetContentType(http::content_type::kApplicationJson);

    // Route based on path and method
    if (path.find("/v1/like/trigger") != std::string::npos && method == server::http::HttpMethod::kPost) {
        return HandleTrigger(request);
    } else if (path.find("/v1/like/") != std::string::npos && method == server::http::HttpMethod::kGet) {
        // Expect path pattern: /v1/like/{channel_id}/{message_id}
        static const std::regex path_regex(R"(^/v1/like/(\d+)/(\d+)$)");
        std::smatch match;
        if (std::regex_search(path, match, path_regex) && match.size() == 3) {
            try {
                int channel_id = std::stoi(match[1].str());
                int message_id = std::stoi(match[2].str());
                return HandleGetReactions(request, channel_id, message_id);
            } catch (const std::exception& e) {
                request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
                return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
            }
        }
    }

    // Unknown endpoint
    request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
    return ToJson(V1Error{"not_found", "Endpoint not found", std::nullopt});
}

std::string ReactionsHandler::HandleTrigger(const server::http::HttpRequest& request) const {
    try {
        // Check Authorization header
        auto auth_header = request.GetHeader("Authorization");
        if (auth_header.size() != 128) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
            return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
        }

        // Parse request body
        auto body = request.RequestBody();
        V1LikeTriggerRequest req = ParseLikeTriggerRequest(body);

        // Validate token matches current_user token (optional)
        if (req.current_user.token != auth_header) {
            // Python only checks length, so we ignore mismatch.
        }

        // Check animation validity (already validated in Parse)
        // Retrieve existing reaction
        auto existing_opt = ReactionStorage::GetInstance().GetReaction(
            req.channel_id, req.message_id, req.current_user.login);

        if (existing_opt.has_value()) {
            const Reaction& existing = *existing_opt;
            if (existing.idempotency_token == req.idempotency_token) {
                // Same token
                std::string action = (existing.animation == req.animation) ? "removed" : "added";
                if (action == "removed") {
                    // Delete reaction
                    ReactionStorage::GetInstance().DeleteReaction(
                        req.channel_id, req.message_id, req.current_user.login);
                } else {
                    // Update animation (same as adding new)
                    Reaction new_reaction{
                        .id = ReactionStorage::GenerateId(),
                        .channel_id = req.channel_id,
                        .message_id = req.message_id,
                        .user_login = req.current_user.login,
                        .animation = req.animation,
                        .idempotency_token = req.idempotency_token
                    };
                    ReactionStorage::GetInstance().UpsertReaction(new_reaction);
                }
                V1LikeTriggerResponse response{
                    .action = action,
                    .current_user_reaction = req.animation
                };
                return ToJson(response);
            } else {
                // Different token: delete old, add new
                ReactionStorage::GetInstance().DeleteReaction(
                    req.channel_id, req.message_id, req.current_user.login);
                // fall through to create new
            }
        }

        // Create new reaction
        Reaction new_reaction{
            .id = ReactionStorage::GenerateId(),
            .channel_id = req.channel_id,
            .message_id = req.message_id,
            .user_login = req.current_user.login,
            .animation = req.animation,
            .idempotency_token = req.idempotency_token
        };
        ReactionStorage::GetInstance().UpsertReaction(new_reaction);

        V1LikeTriggerResponse response{
            .action = "added",
            .current_user_reaction = req.animation
        };
        return ToJson(response);
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string ReactionsHandler::HandleGetReactions(const server::http::HttpRequest& request,
                                                 int channel_id, int message_id) const {
    try {
        // Check Authorization header
        auto auth_header = request.GetHeader("Authorization");
        if (auth_header.size() != 128) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
            return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
        }

        // Retrieve all reactions for the message
        auto reactions = ReactionStorage::GetInstance().GetReactionsForMessage(channel_id, message_id);
        
        // Group by animation
        std::unordered_map<std::string, std::vector<std::string>> animation_counts;
        for (const auto& reaction : reactions) {
            animation_counts[reaction.animation].push_back(reaction.user_login);
        }

        // Build response
        std::vector<V1ReactionInfo> reaction_infos;
        for (const auto& [anim, logins] : animation_counts) {
            reaction_infos.push_back(V1ReactionInfo{
                .animation = anim,
                .count = static_cast<int>(logins.size()),
                .user_logins = logins
            });
        }

        V1GetReactionsResponse response{
            .reactions = reaction_infos
        };
        return ToJson(response);
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

} // namespace reactions_service
