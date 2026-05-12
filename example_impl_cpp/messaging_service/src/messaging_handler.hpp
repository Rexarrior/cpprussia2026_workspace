#pragma once

#include <userver/components/component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utest/using_namespace_userver.hpp>

namespace messaging_service {

class MessagingHandler final : public server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-messaging";

    MessagingHandler(const components::ComponentConfig& config,
                     const components::ComponentContext& context);

    std::string HandleRequest(server::http::HttpRequest& request,
                              server::request::RequestContext&) const override;

private:
    std::string HandleChannelMessages(const server::http::HttpRequest& request) const;
    std::string HandleNewMessage(const server::http::HttpRequest& request) const;
    std::string HandleDirectGetOrCreate(const server::http::HttpRequest& request) const;
    std::string HandleDirectList(const server::http::HttpRequest& request) const;
};

} // namespace messaging_service
