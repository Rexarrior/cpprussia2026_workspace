cmake_minimum_required(VERSION 3.14)
project(auth_service CXX)

# Object library for auth handler
add_library(${PROJECT_NAME}_objs OBJECT
    src/auth_handler.hpp
    src/auth_handler.cpp
    src/models.hpp
    src/models.cpp
    src/schemas.hpp
    src/schemas.cpp
)
target_link_libraries(${PROJECT_NAME}_objs userver::core OpenSSL::SSL)
target_include_directories(${PROJECT_NAME}_objs PUBLIC src include)

# Main executable
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_objs)

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR BUILD_TESTING)
    add_executable(${PROJECT_NAME}_tests
        tests/test_auth_handler.cpp
    )
    target_link_libraries(${PROJECT_NAME}_tests ${PROJECT_NAME}_objs userver::utest)
    
    userver_testsuite_add(${PROJECT_NAME}_tests)
endif()