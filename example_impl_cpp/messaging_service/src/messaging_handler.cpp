#include "messaging_handler.hpp"
#include "models.hpp"
#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/http/http_response.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace messaging_service {

MessagingHandler::MessagingHandler(const components::ComponentConfig& config,
                                   const components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
}

std::string MessagingHandler::HandleRequest(server::http::HttpRequest& request,
                                            server::request::RequestContext&) const {
    const auto& path = request.GetRequestPath();
    [[maybe_unused]] const auto& method = request.GetMethod();

    // Routing based on path prefixes
    if (path.find("/channel/message/by-timestamp") != std::string::npos) {
        return HandleChannelMessages(request);
    } else if (path.find("/channel/message/new") != std::string::npos) {
        return HandleNewMessage(request);
    } else if (path.find("/direct/get-or-create") != std::string::npos) {
        return HandleDirectGetOrCreate(request);
    } else if (path.find("/direct/list") != std::string::npos) {
        return HandleDirectList(request);
    }

    // Default: not found
    request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
    return ToJson(V1Error{"not_found", "Endpoint not found", std::nullopt});
}

std::string MessagingHandler::HandleChannelMessages(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseChannelMessageByTimestampRequest(request.RequestBody());
        
        std::lock_guard<std::mutex> lock(db_mutex);
        // Check channel existence
        bool channel_exists = false;
        if (std::holds_alternative<int>(req.channel_id)) {
            int channel_id = std::get<int>(req.channel_id);
            channel_exists = channels_db.find(channel_id) != channels_db.end();
        } else {
            std::string channel_id = std::get<std::string>(req.channel_id);
            channel_exists = direct_channels_db.find(channel_id) != direct_channels_db.end();
        }
        if (!channel_exists) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
            return ToJson(V1Error{"channel_not_found", "Channel does not exist", std::nullopt});
        }
        
        // Filter messages
        std::vector<V1ChannelMessage> matching_messages;
        for (const auto& [msg_id, msg] : messages_db) {
            // Check channel_id match
            bool channel_match = false;
            if (std::holds_alternative<int>(req.channel_id) && std::holds_alternative<int>(msg.channel_id)) {
                channel_match = std::get<int>(req.channel_id) == std::get<int>(msg.channel_id);
            } else if (std::holds_alternative<std::string>(req.channel_id) && std::holds_alternative<std::string>(msg.channel_id)) {
                channel_match = std::get<std::string>(req.channel_id) == std::get<std::string>(msg.channel_id);
            }
            if (!channel_match) continue;
            
            // Check timestamp range
            if (msg.timestamp < req.from) continue;
            if (req.to.has_value() && msg.timestamp > *req.to) continue;
            
            V1ChannelMessage v1msg{
                .current_user = V1CurrentUser{auth_header, msg.user_login, msg.user_name},
                .id = msg.id,
                .timestamp = msg.timestamp,
                .message = msg.message
            };
            matching_messages.push_back(v1msg);
        }
        
        // Sort by timestamp ascending
        std::sort(matching_messages.begin(), matching_messages.end(),
                 [](const V1ChannelMessage& a, const V1ChannelMessage& b) {
                     return a.timestamp < b.timestamp;
                 });
        
        // Apply limit (Python implementation doesn't paginate, but we respect limit)
        if (matching_messages.size() > static_cast<size_t>(req.limit)) {
            matching_messages.resize(req.limit);
        }
        
        V1ChannelMessageByTimestampResponse response{
            .messages = matching_messages,
            .next_cursor = std::nullopt,
            .has_more = false
        };
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string MessagingHandler::HandleNewMessage(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseChannelMessageNewRequest(request.RequestBody());
        
        std::lock_guard<std::mutex> lock(db_mutex);
        // Check channel existence
        bool channel_exists = false;
        if (std::holds_alternative<int>(req.channel_id)) {
            int channel_id = std::get<int>(req.channel_id);
            channel_exists = channels_db.find(channel_id) != channels_db.end();
        } else {
            std::string channel_id = std::get<std::string>(req.channel_id);
            channel_exists = direct_channels_db.find(channel_id) != direct_channels_db.end();
        }
        if (!channel_exists) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
            return ToJson(V1Error{"channel_not_found", "Channel does not exist", std::nullopt});
        }
        
        // Generate message ID
        int message_id = generate_message_id();
        
        // Generate ISO8601 timestamp (UTC)
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
        gmtime_r(&time_t, &tm_buf);
        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
        std::string timestamp = oss.str();
        
        // Create and store message
        Message msg{
            .id = message_id,
            .channel_id = req.channel_id,
            .user_login = req.current_user.login,
            .user_name = req.current_user.name,
            .message = req.message,
            .timestamp = timestamp
        };
        messages_db[message_id] = msg;
        
        V1ChannelMessageNewResponse response{.message_id = message_id};
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string MessagingHandler::HandleDirectGetOrCreate(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseDirectGetOrCreateRequest(request.RequestBody());
        
        // Prevent self-messaging
        if (req.current_user.login == req.other_user_login) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
            return ToJson(V1Error{"bad_request", "Cannot create direct chat with yourself", std::nullopt});
        }
        
        std::lock_guard<std::mutex> lock(db_mutex);
        // Generate deterministic channel ID
        std::string channel_id = get_direct_channel_id(req.current_user.login, req.other_user_login);
        
        // Check if channel already exists
        auto it = direct_channels_db.find(channel_id);
        if (it == direct_channels_db.end()) {
            // Create new direct channel
            DirectChannel channel{
                .channel_id = channel_id,
                .user1 = req.current_user.login,
                .user2 = req.other_user_login
            };
            direct_channels_db[channel_id] = channel;
        }
        
        // Return response (other_user name defaults to login)
        V1DirectGetOrCreateResponse response{
            .channel_id = channel_id,
            .other_user = V1PublicUser{req.other_user_login, req.other_user_login}
        };
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string MessagingHandler::HandleDirectList(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseDirectListRequest(request.RequestBody());
        
        std::lock_guard<std::mutex> lock(db_mutex);
        std::vector<V1DirectChatInfo> chats;
        
        // Find all direct channels involving current user
        for (const auto& [channel_id, channel] : direct_channels_db) {
            std::string other_login;
            if (channel.user1 == req.current_user.login) {
                other_login = channel.user2;
            } else if (channel.user2 == req.current_user.login) {
                other_login = channel.user1;
            } else {
                continue; // not a participant
            }
            
            // Find last message in this channel
            std::optional<V1ChannelMessage> last_msg;
            int last_msg_id = -1;
            std::string last_timestamp;
            for (const auto& [msg_id, msg] : messages_db) {
                if (!std::holds_alternative<std::string>(msg.channel_id)) continue;
                if (std::get<std::string>(msg.channel_id) != channel_id) continue;
                
                // Keep the most recent by timestamp (assuming ISO8601 string comparison works)
                if (last_timestamp.empty() || msg.timestamp > last_timestamp) {
                    last_timestamp = msg.timestamp;
                    last_msg_id = msg_id;
                }
            }
            
            if (last_msg_id != -1) {
                const Message& msg = messages_db.at(last_msg_id);
                last_msg = V1ChannelMessage{
                    .current_user = V1CurrentUser{auth_header, msg.user_login, msg.user_name},
                    .id = msg.id,
                    .timestamp = msg.timestamp,
                    .message = msg.message
                };
            }
            
            chats.push_back(V1DirectChatInfo{
                .channel_id = channel_id,
                .other_user = V1PublicUser{other_login, other_login}, // name defaults to login
                .last_message = last_msg
            });
        }
        
        V1DirectListResponse response{.direct_chats = chats};
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

} // namespace messaging_service
