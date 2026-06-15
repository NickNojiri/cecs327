cmake_minimum_required(VERSION 3.22)
project(key_value_client_binary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
    main.cpp
    src/SocketUtils.cpp
    src/types.cpp
    src/RemoteListStub.cpp
    src/RemoteKeyValueStore.cpp
)

add_executable(key_value_client_binary ${SOURCES})
