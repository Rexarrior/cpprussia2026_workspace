#include <userver/utest/utest.hpp>

#include <userver/components/component_list.hpp>
#include <userver/components/minimal_component_list.hpp>
#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

#include "src/auth_handler.hpp"

namespace auth_service {

namespace {

constexpr std::string_view kConfig = R"(
components_manager:
  task_processors:
    main-task-processor:
      worker_threads: 1
  components:
    server:
      listener:
        port: 8080
        task_processor: main-task-processor
    logging:
      fs-task-processor: main-task-processor
      loggers:
        default:
          file_path: '@stderr'
          level: debug
    handler-auth:
      path: /v1/user
      task_processor: main-task-processor
)";

} // namespace

UTEST(AuthHandler, RegistrationEndpoint) {
    // TODO: implement test
    EXPECT_TRUE(true);
}

UTEST(AuthHandler, AuthorizationEndpoint) {
    // TODO: implement test
    EXPECT_TRUE(true);
}

UTEST(AuthHandler, SearchEndpoint) {
    // TODO: implement test
    EXPECT_TRUE(true);
}

} // namespace auth_service