cmake_minimum_required(VERSION 3.22)
project(key_value_client_binary)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
    main.cpp
        src/SocketUtils.cpp
    src/types.cpp
        src/RemoteListStub.cpp
        src/RemoteList.cpp
)

add_executable(key_value_client_binary ${SOURCES})
