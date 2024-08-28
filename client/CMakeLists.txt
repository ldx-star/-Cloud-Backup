cmake_minimum_required(VERSION 3.29)
project(Client)

set(CMAKE_CXX_STANDARD 11)

include_directories("/home/ldx/cpp-httplib")
add_executable(Client ../Client.cpp)
target_link_libraries(${PROJECT_NAME} boost_filesystem pthread)