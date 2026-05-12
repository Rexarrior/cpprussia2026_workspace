#pragma once

#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utest/using_namespace_userver.hpp>

namespace reactions_service {

class ReactionsHandler final : public server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-reactions";

    ReactionsHandler(const components::ComponentConfig& config,
                     const components::ComponentContext& context);

    std::string HandleRequest(server::http::HttpRequest& request,
                              server::request::RequestContext&) const override;

private:
    std::string HandleTrigger(const server::http::HttpRequest& request) const;
    std::string HandleGetReactions(const server::http::HttpRequest& request, int channel_id, int message_id) const;
};

} // namespace reactions_service
