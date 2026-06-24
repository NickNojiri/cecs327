cmake_minimum_required(VERSION 3.22)
project(list_server_binary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_executable(list_server_binary
    src/main.cpp
    src/RequestHandlers.cpp
    src/ResponseHandlers.cpp
        src/demo.cpp
        include/NameServerHandlers.h
        src/NameServerHandlers.cpp
        include/Messaging.h
        src/Messaging.cpp
)

target_link_libraries(list_server_binary PRIVATE Threads::Threads)