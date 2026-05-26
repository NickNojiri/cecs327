#include <unistd.h>
#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    const char* clientToServer{"/tmp/client_to_server"};
    const char* serverToClient{"/tmp/server_to_client"};

    int writeFd{open(clientToServer, O_WRONLY)};
    if (writeFd == -1) {
        std::cerr << "client open write FIFO failed: " << std::strerror(errno) << '\n';
        std::cerr << "Make sure the server is running first.\n";
        return 1;
    }

    std::string message{"Hello server. This is the client."};

    ssize_t bytesWritten{write(writeFd, message.c_str(), message.size())};
    if (bytesWritten == -1) {
        std::cerr << "client write failed: " << std::strerror(errno) << '\n';
        close(writeFd);
        return 1;
    }

    close(writeFd);

    int readFd{open(serverToClient, O_RDONLY)};
    if (readFd == -1) {
        std::cerr << "client open read FIFO failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    char buffer[256]{};
    ssize_t bytesRead{read(readFd, buffer, sizeof(buffer) - 1)};
    if (bytesRead == -1) {
        std::cerr << "client read failed: " << std::strerror(errno) << '\n';
        close(readFd);
        return 1;
    }

    close(readFd);

    std::cout << "Client received: " << buffer << '\n';

    return 0;
}