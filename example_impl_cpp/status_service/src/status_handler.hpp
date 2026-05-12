#pragma once

#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utest/using_namespace_userver.hpp>

namespace status_service {

class StatusHandler final : public server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-status";

    StatusHandler(const components::ComponentConfig& config,
                  const components::ComponentContext& context);

    std::string HandleRequest(server::http::HttpRequest& request,
                              server::request::RequestContext&) const override;

private:
    std::string HandleUpdate(const server::http::HttpRequest& request) const;
    std::string HandleGetByLogin(const server::http::HttpRequest& request) const;
};

} // namespace status_service
