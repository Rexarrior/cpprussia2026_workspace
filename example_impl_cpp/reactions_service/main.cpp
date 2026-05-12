#include <userver/components/minimal_server_component_list.hpp>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/utils/daemon_run.hpp>

#include <userver/utest/using_namespace_userver.hpp>

#include "src/reactions_handler.hpp"

int main(int argc, char* argv[]) {
    auto component_list = components::MinimalServerComponentList()
        .Append<reactions_service::ReactionsHandler>()
        .Append<clients::dns::Component>()
        .AppendComponentList(clients::http::ComponentList());

    return utils::DaemonMain(argc, argv, component_list);
}
