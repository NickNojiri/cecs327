cmake_minimum_required(VERSION 3.22)
project(key_value_server_binary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_executable(key_value_server_binary
    src/main.cpp
    src/RequestHandlers.cpp
    src/ResponseHandlers.cpp
)

target_link_libraries(key_value_server_binary PRIVATE Threads::Threads)
