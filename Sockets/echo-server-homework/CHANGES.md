# Homework 1: Echo Echo Echo — What I Changed

## Overview

I copied the original `echo-server` project into a new folder called `echo-server-homework`
and modified `main.cpp` to respond differently depending on what the client sends.

The original server simply echoed every message back as-is.
The new server parses the message and picks one of 5 responses.

---

## Files Created

| File | Description |
|------|-------------|
| `CMakeLists.txt` | Build config (same as original, just renamed target to `echo_server_homework`) |
| `main.cpp` | Modified server with all new command logic |

---

## Changes to main.cpp

### 1. New `#include` headers (top of file)

```cpp
#include <chrono>    // for std::chrono::system_clock
#include <ctime>     // for std::time_t, std::tm, localtime_r
#include <iomanip>   // for std::setw, std::setfill
#include <sstream>   // for std::ostringstream
```

These are needed to get and format the current time for the `TIME` command.

---

### 2. New function: `get_current_time()`

```cpp
std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm{};
    localtime_r(&now_t, &local_tm);   // Linux/WSL version (thread-safe)
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << local_tm.tm_hour << ":"
        << std::setw(2) << local_tm.tm_min  << ":"
        << std::setw(2) << local_tm.tm_sec;
    return oss.str();   // e.g. "09:15:00"
}
```

Gets the current server time and formats it as `hh:mm:ss`.

---

### 3. New function: `build_response()`

This is the main logic that decides what to send back to the client.

```cpp
std::string build_response(const std::string& message, bool& should_quit)
```

| Client sends | Condition | Server responds |
|---|---|---|
| `ECHO Hello!` | `message` starts with `"ECHO"` | ` Hello!\n` (everything after ECHO) |
| `TIME` | `message` starts with `"TIME"` | `09:15:00\n` (current time) |
| `HELP` | exact match after trimming | List of all supported commands |
| `QUIT` | exact match after trimming | `GOODBYE\n` then closes connection |
| Anything else | no match | `ERROR unknown command\n` |

The `should_quit` flag is set to `true` when `QUIT` is received so the
main loop knows to break and close the socket.

---

### 4. Modified the message loop in `main()`

**Before (original):**
```cpp
// Respond to the client with the same message.
ssize_t bytes_sent{send(client_fd, message.c_str(), message.size(), 0)};
```

**After (new):**
```cpp
// Determine the appropriate response.
bool should_quit{false};
std::string response{build_response(message, should_quit)};

// Send the response to the client.
ssize_t bytes_sent{send(client_fd, response.c_str(), response.size(), 0)};

// If QUIT was received, close the client connection gracefully.
if (should_quit) {
    std::cout << "Client requested QUIT. Closing connection.\n";
    break;
}
```

Instead of sending the raw message back, we now call `build_response()`
to pick the right reply, and break out of the loop on QUIT.

---

## How to Build and Run (WSL)

**Terminal 1 — Server:**
```bash
cd /mnt/c/Users/17143/Desktop/327/Lectures/Sockets/echo-server-homework
g++ -std=c++20 -o echo_server_homework main.cpp
./echo_server_homework
```

**Terminal 2 — Client (already built):**
```bash
/mnt/c/Users/17143/Desktop/327/Lectures/Sockets/echo-client/build/echo_client
```

**Test commands to type in the client:**
```
ECHO Hello World
TIME
HELP
QUIT
```
