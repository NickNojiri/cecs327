#include "RequestHandlers.h"
#include "ResponseHandlers.h"

std::vector<std::uint8_t> handlePutRequest(MessageReader& reader, SharedStore& store) {
    std::optional<std::string> key{reader.readString()};
    std::optional<std::string> value{reader.readString()};
    if (!key.has_value() || !value.has_value() || !reader.isAtEnd()) {
        return buildErrorResponse("PUT requires key and value");
    }

    {
        std::lock_guard<std::mutex> lock{store.mutex};
        store.values[key.value()] = value.value();
    }

    return buildStatusResponse(ResponseOpcode::Ok);
}

std::vector<std::uint8_t> handleGetRequest(MessageReader& reader, SharedStore& store) {
    std::optional<std::string> key{reader.readString()};
    if (!key.has_value() || !reader.isAtEnd()) {
        return buildErrorResponse("GET requires key");
    }

    std::lock_guard<std::mutex> lock{store.mutex};
    auto it = store.values.find(key.value());
    if (it == store.values.end()) {
        return buildStatusResponse(ResponseOpcode::NotFound);
    }

    return buildValueResponse(it->second);
}

std::vector<std::uint8_t> handleDeleteRequest(MessageReader& reader, SharedStore& store) {
    std::optional<std::string> key{reader.readString()};
    if (!key.has_value() || !reader.isAtEnd()) {
        return buildErrorResponse("DELETE requires key");
    }

    std::lock_guard<std::mutex> lock{store.mutex};
    auto erased = store.values.erase(key.value());
    if (erased == 0) {
        return buildStatusResponse(ResponseOpcode::NotFound);
    }

    return buildStatusResponse(ResponseOpcode::Ok);
}

std::vector<std::uint8_t> handleCountRequest(MessageReader& reader, SharedStore& store) {
    if (!reader.isAtEnd()) {
        return buildErrorResponse("COUNT takes no arguments");
    }

    std::lock_guard<std::mutex> lock{store.mutex};
    return buildCountResponse(store.values.size());
}

std::vector<std::uint8_t> handleExistsRequest(MessageReader& reader, SharedStore& store) {
    std::optional<std::string> key{reader.readString()};
    if (!key.has_value() || !reader.isAtEnd()) {
        return buildErrorResponse("EXISTS requires key");
    }

    std::lock_guard<std::mutex> lock{store.mutex};
    if (store.values.count(key.value()) > 0) {
        return buildStatusResponse(ResponseOpcode::Ok);
    }

    return buildStatusResponse(ResponseOpcode::NotFound);
}

std::vector<std::uint8_t> handleKeysRequest(MessageReader& reader, SharedStore& store) {
    if (!reader.isAtEnd()) {
        return buildErrorResponse("KEYS takes no arguments");
    }

    std::lock_guard<std::mutex> lock{store.mutex};
    std::vector<std::string> keys{};
    keys.reserve(store.values.size());
    for (const auto& pair : store.values) {
        keys.push_back(pair.first);
    }

    return buildKeysResponse(keys);
}
