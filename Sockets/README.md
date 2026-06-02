# Sockets Key-Value Store & Command Improvements

This directory contains improvements to the key-value socket-based store. We added support for three new commands (`EXIST`, `CLEAR`, and `KEYS`), corrected client synchronization to handle connection greetings properly, and added a cross-platform Jupyter Notebook (`echo_demo_updated.ipynb`) to test the flow out of the box in Windows (MSYS2) and Linux (GitHub Codespaces).

---

## 1. Key-Value Server Commands Added

The key-value server in [`key-value-server/main.cpp`](./key-value-server/main.cpp) has been updated to handle the following commands:

| Command | Arguments | Behavior | Server Response |
| :--- | :--- | :--- | :--- |
| **`EXIST`** | `<key>` | Checks if a key exists in the memory map | `TRUE` or `FALSE` |
| **`CLEAR`** | *None* | Empties all key-value mappings in the store | `OK` |
| **`KEYS`** | *None* | Lists all keys in a map. | `KEY[key1 key2 ...]` or `EMPTY` if empty |

### Implementation Snippet

```cpp
    if (command.name == "EXIST") {
        if (command.args.size() != 1) {
            return "ERROR EXIST requires key\n";
        }
        const std::string& key{command.args[0]};
        return (store.find(key) != store.end() ? "TRUE\n" : "FALSE\n");
    }

    if (command.name == "CLEAR") {
        if (!command.args.empty()) {
            return "ERROR CLEAR takes no arguments\n";
        }
        store.clear();
        return "OK\n";
    }

    if (command.name == "KEYS") {
        if (!command.args.empty()) {
            return "ERROR KEYS takes no arguments\n";
        }
        if (store.empty()) {
            return "EMPTY\n";
        }
        std::string result = "KEY[";
        bool first = true;
        for (const auto& kv : store) {
            if (!first) result += " ";
            result += kv.first;
            first = false;
        }
        result += "]\n";
        return result;
    }
```

---

## 2. Client Synchronization Fix

The generic echo client in [`echo-client/main.cpp`](./echo-client/main.cpp) previously started prompting the user for input without reading the server's connection greeting (`OK connected`). In a TCP flow, this caused an "off-by-one" response alignment where every printed response belonged to the *previous* command.

We resolved this by reading and printing the connection greeting *immediately* upon connection, before entering the command loop:

```cpp
        // Read connection greeting from the server first
        char greeting_buffer[BUFFER_SIZE]{};
        ssize_t greeting_received{recv(client_fd, greeting_buffer, BUFFER_SIZE - 1, 0)};
        if (greeting_received > 0) {
            greeting_buffer[greeting_received] = '\0';
            std::cout << "Server: " << greeting_buffer;
        }
```

This ensures responses (like `FALSE`, `OK`, and final `BYE` on `QUIT`) align exactly with the commands sent.

---

## 3. Running the Demo

The demo runs inside `echo_demo_updated.ipynb`. It is configured to run out-of-the-box on:
- **Windows (via MSYS2 toolchain):** Detects pathing, compiler, and adds `C:\msys64\usr\bin` to the PATH at runtime to locate `msys-2.0.dll`.
- **Linux / GitHub Codespaces:** Automatically compiles native binaries using the system `cmake` and `gcc`/`make` and executes them natively.
