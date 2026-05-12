#include "notifications_handler.hpp"
#include "models.hpp"
#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/http/http_response.hpp>
#include <fmt/format.h>
#include <userver/logging/log.hpp>

#include <algorithm>

namespace notifications_service {

NotificationsHandler::NotificationsHandler(const components::ComponentConfig& config,
                                           const components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
}

std::string NotificationsHandler::HandleRequest(server::http::HttpRequest& request,
                                                server::request::RequestContext&) const {
    const auto& method = request.GetMethod();
    const auto& action = request.GetPathArg("action");
    LOG_DEBUG() << "NotificationsHandler: action='" << action << "', method=" << static_cast<int>(method);

    if (method == server::http::HttpMethod::kPost) {
        if (action == "list") {
            return HandleList(request);
        } else if (action == "new") {
            return HandleNew(request);
        }
    }

    // Default: not found
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
    return ToJson(V1Error{"not_found", "Endpoint not found", std::nullopt});
}

std::string NotificationsHandler::HandleList(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseListRequest(request.RequestBody());
        
        std::lock_guard<std::mutex> lock(db_mutex);
        std::vector<V1NotificationStatus> user_notifications;
        
        for (const auto& [notification_id, notification] : notifications_db) {
            // Filter by to_user_login and channel_id
            if (notification.to_user_login != req.current_user.login) continue;
            
            // Compare channel_id (variant)
            bool channel_match = false;
            if (std::holds_alternative<int>(req.channel_id) && std::holds_alternative<int>(notification.channel_id)) {
                channel_match = std::get<int>(req.channel_id) == std::get<int>(notification.channel_id);
            } else if (std::holds_alternative<std::string>(req.channel_id) && std::holds_alternative<std::string>(notification.channel_id)) {
                channel_match = std::get<std::string>(req.channel_id) == std::get<std::string>(notification.channel_id);
            }
            if (!channel_match) continue;
            
            user_notifications.push_back(V1NotificationStatus{
                .message_id = notification.message_id,
                .read = notification.read
            });
        }
        
        V1ChannelNotificationListResponse response{.notifications = user_notifications};
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string NotificationsHandler::HandleNew(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseNewRequest(request.RequestBody());
        
        std::lock_guard<std::mutex> lock(db_mutex);
        std::string notification_id = generate_notification_id();
        
        Notification notification{
            .notification_id = notification_id,
            .channel_id = req.channel_id,
            .message_id = req.message_id,
            .from_user_login = req.current_user.login,
            .to_user_login = req.other_user_login,
            .read = false
        };
        
        notifications_db[notification_id] = notification;
        
        V1ChannelNotificationNewResponse response{.notification_id = notification_id};
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

} // namespace notifications_service
