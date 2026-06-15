#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>
#include <limits>

// Response opcodes for the key-value application protocol.
//   Ok       - a status response meaning the operation succeeded (PUT, DELETE,
//              and EXISTS-when-present all use this).
//   Value    - a single string value (the response to GET).
//   Count    - a single integer (the response to COUNT).
//   Keys     - an integer count followed by that many encoded strings
//              (the response to KEYS).
//   NotFound - a status response meaning the key was not present (GET, DELETE,
//              and EXISTS-when-absent use this).
//   Error    - an error string describing a malformed request.
enum class ResponseOpcode : std::uint8_t {
    Ok       = 0x40,
    Value    = 0x41,
    Count    = 0x42,
    Keys     = 0x43,
    NotFound = 0x44,
    Error    = 0x7F
};

std::vector<std::uint8_t> buildStatusResponse(ResponseOpcode opcode);
std::vector<std::uint8_t> buildErrorResponse(const std::string& error);
std::vector<std::uint8_t> buildValueResponse(const std::string& value);
std::vector<std::uint8_t> buildCountResponse(std::size_t count);
std::vector<std::uint8_t> buildKeysResponse(const std::vector<std::string>& keys);

// Shared encoding helpers, also used when building the KEYS payload.
bool appendInt32(std::vector<std::uint8_t>& payload, std::int32_t value);
bool appendString(std::vector<std::uint8_t>& payload, const std::string& value);
