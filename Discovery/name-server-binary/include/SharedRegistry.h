#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct ServiceProvider {
    std::string identifier{};
    std::string host{};
    std::int32_t port{};
    // The moment we last heard from this provider. Used by the name server's
    // reaper thread to discard providers that stop sending heartbeats.
    std::chrono::system_clock::time_point lastHeartbeat{};

    // Whenever a ServiceProvider is constructed, stamp lastHeartbeat with the
    // current system time so a freshly-registered provider starts out "alive".
    ServiceProvider()
        : lastHeartbeat{std::chrono::system_clock::now()} {}

    ServiceProvider(std::string identifier, std::string host, std::int32_t port)
        : identifier{std::move(identifier)},
          host{std::move(host)},
          port{port},
          lastHeartbeat{std::chrono::system_clock::now()} {}
};

struct SharedRegistry {
    std::unordered_map<std::string, std::vector<ServiceProvider>> services{};
    std::mutex mutex{};
};
