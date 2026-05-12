#include "status_handler.hpp"
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
#include <unordered_set>

namespace status_service {

namespace {

const std::unordered_set<std::string> VALID_STATUS_TYPES = {"online", "away", "busy", "offline"};
const std::unordered_set<std::string> VALID_VISIBILITY = {"public", "private"};

std::string TimePointToIsoString(const std::chrono::system_clock::time_point& tp) {
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_buf;
    gmtime_r(&time, &tm_buf);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S") << "Z";
    return oss.str();
}

std::chrono::system_clock::time_point Now() {
    return std::chrono::system_clock::now();
}

} // anonymous namespace

StatusHandler::StatusHandler(const components::ComponentConfig& config,
                             const components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
}

std::string StatusHandler::HandleRequest(server::http::HttpRequest& request,
                                         server::request::RequestContext&) const {
    const auto& path = request.GetRequestPath();
    [[maybe_unused]] const auto& method = request.GetMethod();

    // Routing based on path suffixes
    if (path.find("/update") != std::string::npos) {
        return HandleUpdate(request);
    } else if (path.find("/by-login") != std::string::npos) {
        return HandleGetByLogin(request);
    }

    // Default: not found
    request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
    return ToJson(V1Error{"not_found", "Endpoint not found", std::nullopt});
}

std::string StatusHandler::HandleUpdate(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseUpdateRequest(request.RequestBody());
        
        // Validate token matches (already done in parse)
        
        // Extract status fields from dict
        std::string status_type = "online"; // default
        auto it_type = req.status.find("status_type");
        if (it_type != req.status.end()) {
            status_type = it_type->second;
            if (VALID_STATUS_TYPES.find(status_type) == VALID_STATUS_TYPES.end()) {
                request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
                return ToJson(V1Error{"invalid_status_type",
                    fmt::format("Invalid status_type. Must be one of: online, away, busy, offline"),
                    std::nullopt});
            }
        }
        
        std::string status_message = "";
        auto it_message = req.status.find("status_message");
        if (it_message != req.status.end()) {
            status_message = it_message->second;
        }
        
        std::string visibility = "public"; // default
        auto it_visibility = req.status.find("visibility");
        if (it_visibility != req.status.end()) {
            visibility = it_visibility->second;
            if (VALID_VISIBILITY.find(visibility) == VALID_VISIBILITY.end()) {
                request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
                return ToJson(V1Error{"invalid_visibility",
                    fmt::format("Invalid visibility. Must be one of: public, private"),
                    std::nullopt});
            }
        }
        
        auto now = Now();
        auto expires_at = now + std::chrono::seconds(DEFAULT_TTL_SECONDS);
        
        UserStatus user_status{
            .login = req.current_user.login,
            .status_type = status_type,
            .status_message = status_message,
            .visibility = visibility,
            .updated_at = now,
            .expires_at = expires_at
        };
        
        StatusStorage::GetInstance().SetStatus(user_status);
        
        V1UserStatusUpdateResponse response{
            .success = true,
            .updated_at = TimePointToIsoString(now),
            .expires_at = TimePointToIsoString(expires_at)
        };
        
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string StatusHandler::HandleGetByLogin(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    // Check Authorization header
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.size() != 128) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
        return ToJson(V1Error{"invalid_token", "Invalid token", std::nullopt});
    }
    
    try {
        auto req = ParseByLoginRequest(request.RequestBody());
        
        // Validate token matches (already done in parse)
        
        auto opt_status = StatusStorage::GetInstance().GetStatus(req.login);
        if (!opt_status.has_value()) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
            return ToJson(V1Error{"status_not_found", "User status not found", std::nullopt});
        }
        
        const UserStatus& status = *opt_status;
        
        // Check visibility
        if (status.visibility == "private" && status.login != req.current_user.login) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kForbidden);
            return ToJson(V1Error{"forbidden", "Status is private", std::nullopt});
        }
        
        std::unordered_map<std::string, std::string> status_dict;
        status_dict["status_type"] = status.status_type;
        status_dict["status_message"] = status.status_message;
        status_dict["visibility"] = status.visibility;
        status_dict["updated_at"] = TimePointToIsoString(status.updated_at);
        
        V1UserStatusByLoginResponse response{
            .status = status_dict
        };
        
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

} // namespace status_service
