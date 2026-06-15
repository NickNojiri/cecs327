#pragma once
#include "RemoteListStub.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

// Abstracts a key-value store with a familiar hashmap-like interface, while
// secretly communicating with a key-value-server-binary instance where the real
// store lives. Each method encodes its arguments, sends the matching opcode
// through the stub, and decodes the response.
class RemoteKeyValueStore {
public:
    explicit RemoteKeyValueStore(const std::string& host = "127.0.0.1", int port = 9090);
    ~RemoteKeyValueStore() = default;

    RemoteKeyValueStore(const RemoteKeyValueStore&) = delete;
    RemoteKeyValueStore& operator=(const RemoteKeyValueStore&) = delete;
    RemoteKeyValueStore(RemoteKeyValueStore&&) = default;
    RemoteKeyValueStore& operator=(RemoteKeyValueStore&&) = default;

    // Sends a PUT message; returns true if the response is OK.
    bool put(const std::string& key, const std::string& value);
    // Sends a GET message; returns null if NOT_FOUND, or the string of a VALUE response.
    std::optional<std::string> get(const std::string& key);
    // Sends a DELETE message; returns true if a pair was removed (OK), false otherwise.
    bool remove(const std::string& key);
    // Sends a COUNT message; returns the number of pairs in the store.
    std::optional<std::size_t> count();
    // Sends an EXISTS message; returns true if the response is OK, false if NOT_FOUND.
    bool exists(const std::string& key);
    // Sends a KEYS message; returns the resulting vector of strings.
    std::optional<std::vector<std::string>> keys();

    bool isConnected() const;

private:
    RemoteListStub m_stub;
};
