#pragma once

#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utest/using_namespace_userver.hpp>

namespace notifications_service {

class NotificationsHandler final : public server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-notifications";

    NotificationsHandler(const components::ComponentConfig& config,
                         const components::ComponentContext& context);

    std::string HandleRequest(server::http::HttpRequest& request,
                              server::request::RequestContext&) const override;

private:
    std::string HandleList(const server::http::HttpRequest& request) const;
    std::string HandleNew(const server::http::HttpRequest& request) const;
};

} // namespace notifications_service
