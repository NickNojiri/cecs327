#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    const char* clientToServer{"/tmp/client_to_server"};
    const char* serverToClient{"/tmp/server_to_client"};

    // Create the named pipes if they do not already exist.
    if (mkfifo(clientToServer, 0666) == -1 && errno != EEXIST) {
        std::cerr << "mkfifo clientToServer failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    if (mkfifo(serverToClient, 0666) == -1 && errno != EEXIST) {
        std::cerr << "mkfifo serverToClient failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    std::cout << "Server waiting for client...\n";

    int readFd{open(clientToServer, O_RDONLY)};
    if (readFd == -1) {
        std::cerr << "server open read FIFO failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    char buffer[256]{};
    ssize_t bytesRead{read(readFd, buffer, sizeof(buffer) - 1)};
    if (bytesRead == -1) {
        std::cerr << "server read failed: " << std::strerror(errno) << '\n';
        close(readFd);
        return 1;
    }

    close(readFd);

    std::cout << "Server received: " << buffer << '\n';

    int writeFd{open(serverToClient, O_WRONLY)};
    if (writeFd == -1) {
        std::cerr << "server open write FIFO failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    std::string reply{"Hello client. The server received your message."};

    ssize_t bytesWritten{write(writeFd, reply.c_str(), reply.size())};
    if (bytesWritten == -1) {
        std::cerr << "server write failed: " << std::strerror(errno) << '\n';
        close(writeFd);
        return 1;
    }

    close(writeFd);

    std::cout << "Server done.\n";

    return 0;
}