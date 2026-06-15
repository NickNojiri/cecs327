#include "RequestHandlers.h"
#include "ResponseHandlers.h"
#include <utility>

// PUT: 2 string arguments (key, value). Stores the pair and responds OK.
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

// GET: 1 string argument (key). Responds VALUE with the value if present,
// or NOT_FOUND otherwise.
std::vector<std::uint8_t> handleGetRequest(MessageReader& reader, SharedStore& store) {
    std::optional<std::string> key{reader.readString()};
    if (!key.has_value() || !reader.isAtEnd()) {
        return buildErrorResponse("GET requires key");
    }

    std::string value{};
    {
        std::lock_guard<std::mutex> lock{store.mutex};
        auto it{store.values.find(key.value())};
        if (it == store.values.end()) {
            return buildStatusResponse(ResponseOpcode::NotFound);
        }
        value = it->second;
    }

    return buildValueResponse(value);
}

// DELETE: 1 string argument (key). Responds OK if a pair was removed,
// or NOT_FOUND if the key was not present.
std::vector<std::uint8_t> handleDeleteRequest(MessageReader& reader, SharedStore& store) {
    std::optional<std::string> key{reader.readString()};
    if (!key.has_value() || !reader.isAtEnd()) {
        return buildErrorResponse("DELETE requires key");
    }

    std::size_t removed{};
    {
        std::lock_guard<std::mutex> lock{store.mutex};
        // erase() returns the number of keys that were removed.
        removed = store.values.erase(key.value());
    }

    if (removed == 0) {
        return buildStatusResponse(ResponseOpcode::NotFound);
    }

    return buildStatusResponse(ResponseOpcode::Ok);
}

// COUNT: no arguments. Responds COUNT with the number of pairs in the store.
std::vector<std::uint8_t> handleCountRequest(MessageReader& reader, SharedStore& store) {
    if (!reader.isAtEnd()) {
        return buildErrorResponse("COUNT takes no arguments");
    }

    std::size_t count{};
    {
        std::lock_guard<std::mutex> lock{store.mutex};
        count = store.values.size();
    }

    return buildCountResponse(count);
}

// EXISTS: 1 string argument (key). Responds OK if the key exists,
// or NOT_FOUND otherwise.
std::vector<std::uint8_t> handleExistsRequest(MessageReader& reader, SharedStore& store) {
    std::optional<std::string> key{reader.readString()};
    if (!key.has_value() || !reader.isAtEnd()) {
        return buildErrorResponse("EXISTS requires key");
    }

    bool found{};
    {
        std::lock_guard<std::mutex> lock{store.mutex};
        found = store.values.find(key.value()) != store.values.end();
    }

    return buildStatusResponse(found ? ResponseOpcode::Ok : ResponseOpcode::NotFound);
}

// KEYS: no arguments. Responds KEYS with an integer count of the keys, followed
// by the encoded string for each key.
std::vector<std::uint8_t> handleKeysRequest(MessageReader& reader, SharedStore& store) {
    if (!reader.isAtEnd()) {
        return buildErrorResponse("KEYS takes no arguments");
    }

    std::vector<std::string> keys{};
    {
        std::lock_guard<std::mutex> lock{store.mutex};
        keys.reserve(store.values.size());
        for (const auto& [key, value] : store.values) {
            keys.push_back(key);
        }
    }

    return buildKeysResponse(keys);
}
