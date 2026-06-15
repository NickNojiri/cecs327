#include "RemoteKeyValueStore.h"
#include "types.h"

RemoteKeyValueStore::RemoteKeyValueStore(const std::string& host, int port)
    : m_stub{host, port} {
}

// PUT key value -> OK. We encode the key and value strings (each length-prefixed),
// send a PUT, and report whether the server answered OK.
bool RemoteKeyValueStore::put(const std::string& key, const std::string& value) {
    std::vector<std::uint8_t> arguments{};
    if (!appendString(arguments, key) || !appendString(arguments, value)) {
        return false;
    }

    auto response = m_stub.sendRequest(RequestOpcode::Put, arguments);
    if (!response.has_value()) {
        return false;
    }

    return parseStatusResponse(response.value());
}

// GET key -> VALUE or NOT_FOUND. parseValueResponse yields the string for a VALUE
// response and null for anything else (including NOT_FOUND), which is exactly the
// behavior we want.
std::optional<std::string> RemoteKeyValueStore::get(const std::string& key) {
    std::vector<std::uint8_t> arguments{};
    if (!appendString(arguments, key)) {
        return std::nullopt;
    }

    auto response = m_stub.sendRequest(RequestOpcode::Get, arguments);
    if (!response.has_value()) {
        return std::nullopt;
    }

    return parseValueResponse(response.value());
}

// DELETE key -> OK if a pair was removed, NOT_FOUND otherwise.
bool RemoteKeyValueStore::remove(const std::string& key) {
    std::vector<std::uint8_t> arguments{};
    if (!appendString(arguments, key)) {
        return false;
    }

    auto response = m_stub.sendRequest(RequestOpcode::Delete, arguments);
    if (!response.has_value()) {
        return false;
    }

    return parseStatusResponse(response.value());
}

// COUNT -> the number of pairs in the store.
std::optional<std::size_t> RemoteKeyValueStore::count() {
    auto response = m_stub.sendRequest(RequestOpcode::Count, {});
    if (!response.has_value()) {
        return std::nullopt;
    }

    return parseCountResponse(response.value());
}

// EXISTS key -> OK if present (true), NOT_FOUND if absent (false).
bool RemoteKeyValueStore::exists(const std::string& key) {
    std::vector<std::uint8_t> arguments{};
    if (!appendString(arguments, key)) {
        return false;
    }

    auto response = m_stub.sendRequest(RequestOpcode::Exists, arguments);
    if (!response.has_value()) {
        return false;
    }

    return parseStatusResponse(response.value());
}

// KEYS -> the vector of all keys currently in the store.
std::optional<std::vector<std::string>> RemoteKeyValueStore::keys() {
    auto response = m_stub.sendRequest(RequestOpcode::Keys, {});
    if (!response.has_value()) {
        return std::nullopt;
    }

    return parseKeysResponse(response.value());
}

bool RemoteKeyValueStore::isConnected() const {
    return m_stub.isConnected();
}
