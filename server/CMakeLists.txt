cmake_minimum_required(VERSION 3.29)
project(Server)

set(CMAKE_CXX_STANDARD 11)

include_directories("/home/ldx/bundle" "/home/ldx/cpp-httplib")
add_library(lib "/home/ldx/bundle/bundle.cpp" )
add_executable(Server ../Server.cpp)
target_link_libraries(${PROJECT_NAME} lib boost_filesystem jsoncpp pthread)