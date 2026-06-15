#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

// The hashmap is a critical resource: only one thread can access it at a time,
// or else race conditions occur and the map can be corrupted. We wrap the map
// and its mutual-exclusion lock together in a single SharedStore object so both
// can be handed to each client thread.
struct SharedStore {
    std::unordered_map<std::string, std::string> values{};
    std::mutex mutex{};
};
