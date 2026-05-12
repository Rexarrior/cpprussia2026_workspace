#pragma once

#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utest/using_namespace_userver.hpp>

namespace auth_service {

class AuthHandler final : public server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-auth";

    AuthHandler(const components::ComponentConfig& config,
                const components::ComponentContext& context);

    std::string HandleRequest(server::http::HttpRequest& request,
                              server::request::RequestContext&) const override;

private:
    std::string HandleRegistration(const server::http::HttpRequest& request) const;
    std::string HandleAuthorization(const server::http::HttpRequest& request) const;
    std::string HandleSearch(const server::http::HttpRequest& request) const;
};

} // namespace auth_service