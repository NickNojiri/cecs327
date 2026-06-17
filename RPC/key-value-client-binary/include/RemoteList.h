#pragma once
#include "RemoteListStub.h"
#include <optional>
#include <string>
#include <vector>

class RemoteKeyValueStore {
public:
    explicit RemoteKeyValueStore(const std::string& host = "127.0.0.1", int port = 9090);
    ~RemoteKeyValueStore() = default;

    RemoteKeyValueStore(const RemoteKeyValueStore&) = delete;
    RemoteKeyValueStore& operator=(const RemoteKeyValueStore&) = delete;
    RemoteKeyValueStore(RemoteKeyValueStore&&) = default;
    RemoteKeyValueStore& operator=(RemoteKeyValueStore&&) = default;

    bool put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool exists(const std::string& key);
    std::optional<std::vector<std::string>> keys();
    bool isConnected() const;

private:
    RemoteListStub m_stub;
};
