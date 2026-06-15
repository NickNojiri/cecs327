#ifndef REMOTE_LIST_STUB_H
#define REMOTE_LIST_STUB_H

#include "types.h"
#include <optional>
#include <string>
#include <vector>

// A helper class for sending requests and getting back responses from a server.
// It only knows how to frame an opcode plus arguments, send them, and parse the
// framed response back into a BinaryResponse -- it has no idea what the opcodes
// mean. RemoteKeyValueStore layers the key-value semantics on top of it.
class RemoteListStub {
public:
    RemoteListStub(const std::string& host, int port);
    ~RemoteListStub();

    RemoteListStub(const RemoteListStub&) = delete;
    RemoteListStub& operator=(const RemoteListStub&) = delete;

    std::optional<BinaryResponse> sendRequest(RequestOpcode opcode,
                                              const std::vector<std::uint8_t>& arguments);
    bool isConnected() const;
    int getSocket() const { return socket_fd_; }

private:
    int socket_fd_;
};

#endif // REMOTE_LIST_STUB_H
