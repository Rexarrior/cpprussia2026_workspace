#include "auth_handler.hpp"
#include "models.hpp"
#include "schemas.hpp"

#include <userver/formats/json.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/server/http/http_response.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <cctype>

namespace auth_service {

AuthHandler::AuthHandler(const components::ComponentConfig& config,
                         const components::ComponentContext& context)
    : HttpHandlerBase(config, context) {
}

std::string AuthHandler::HandleRequest(server::http::HttpRequest& request,
                                        server::request::RequestContext&) const {
    const auto& method = request.GetMethod();
    const auto& action = request.GetPathArg("action");
    LOG_DEBUG() << "AuthHandler: action='" << action << "', method=" << static_cast<int>(method);

    if (method == server::http::HttpMethod::kPost) {
        if (action == "registration") {
            return HandleRegistration(request);
        } else if (action == "authorization") {
            return HandleAuthorization(request);
        } else if (action == "search") {
            return HandleSearch(request);
        }
    }

    // Default: method not allowed or not found
    request.GetHttpResponse().SetStatus(server::http::HttpStatus::kNotFound);
    return ToJson(V1Error{"not_found", "Endpoint not found", std::nullopt});
}

std::string AuthHandler::HandleRegistration(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    try {
        auto req = ParseRegistrationRequest(request.RequestBody());
        
        // Check if user already exists
        std::lock_guard<std::mutex> lock(db_mutex);
        if (users_db.find(req.login) != users_db.end()) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kConflict);
            return ToJson(V1Error{"user_exists", fmt::format("User {} already exists", req.login), std::nullopt});
        }
        
        // Create user
        auto token = generate_token();
        User user{
            .login = req.login,
            .name = req.name,
            .email = req.email,
            .phone = req.phone,
            .password_hash = hash_password(req.password),
            .token = token
        };
        
        users_db[req.login] = user;
        tokens_db[token] = req.login;
        
        V1UserAuthorizationResponse response{
            .current_user = V1AuthorizedUser{token, user.login, user.name}
        };
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string AuthHandler::HandleAuthorization(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    try {
        auto req = ParseAuthorizationRequest(request.RequestBody());
        
        std::lock_guard<std::mutex> lock(db_mutex);
        auto it = users_db.find(req.login);
        if (it == users_db.end() || !verify_password(req.password, it->second.password_hash)) {
            request.GetHttpResponse().SetStatus(server::http::HttpStatus::kUnauthorized);
            return ToJson(V1Error{"invalid_credentials", "Invalid login or password", std::nullopt});
        }
        
        // Invalidate old token
        if (it->second.token) {
            tokens_db.erase(*it->second.token);
        }
        
        // Generate new token
        auto token = generate_token();
        it->second.token = token;
        tokens_db[token] = it->second.login;
        
        V1UserAuthorizationResponse response{
            .current_user = V1AuthorizedUser{token, it->second.login, it->second.name}
        };
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

std::string AuthHandler::HandleSearch(const server::http::HttpRequest& request) const {
    request.GetHttpResponse().SetContentType(userver::http::content_type::kApplicationJson);
    
    try {
        std::lock_guard<std::mutex> lock(db_mutex);
        auto req = ParseSearchRequest(request.RequestBody());
        
        std::vector<V1PublicUser> matching_users;
        std::string query_lower = req.query;
        std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        
        for (const auto& [login, user] : users_db) {
            std::string login_lower = login;
            std::transform(login_lower.begin(), login_lower.end(), login_lower.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            std::string name_lower = user.name;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            
            if (login_lower.find(query_lower) != std::string::npos ||
                name_lower.find(query_lower) != std::string::npos) {
                matching_users.push_back(V1PublicUser{login, user.name});
                if (matching_users.size() >= static_cast<size_t>(req.limit)) {
                    break;
                }
            }
        }
        
        V1UserSearchResponse response{.users = matching_users};
        return ToJson(response);
        
    } catch (const std::exception& e) {
        request.GetHttpResponse().SetStatus(server::http::HttpStatus::kBadRequest);
        return ToJson(V1Error{"bad_request", e.what(), std::nullopt});
    }
}

} // namespace auth_service