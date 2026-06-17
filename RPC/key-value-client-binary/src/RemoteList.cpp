#include "RemoteList.h"
#include "types.h"

RemoteKeyValueStore::RemoteKeyValueStore(const std::string& host, int port)
    : m_stub{host, port} {
}

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
