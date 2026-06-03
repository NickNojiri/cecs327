#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>

namespace {
    constexpr int PORT{9090};
    constexpr int BACKLOG{5};
    constexpr int BUFFER_SIZE{1024};
    constexpr int MAX_CLIENTS{3};

    void close_socket(int fd) {
        if (fd >= 0) {
            close(fd);
        }
    }

    void fail(const std::string& message) {
        throw std::runtime_error{message + ": " + std::strerror(errno)};
    }
}

void handleClient(int client_fd, int& client_count, std::mutex& client_mutex) {
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        client_count++;
        std::cout << "Client connected. Total clients: " << client_count << "\n";
    }

    char buffer[BUFFER_SIZE]{};

    while (true) {
        ssize_t bytes_received{recv(client_fd, buffer, BUFFER_SIZE - 1, 0)};

        if (bytes_received < 0) {
            std::cerr << "recv failed: " << std::strerror(errno) << "\n";
            break;
        }

        if (bytes_received == 0) {
            std::cout << "Client disconnected.\n";
            break;
        }

        buffer[bytes_received] = '\0';
        std::string message{buffer};

        while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) {
            message.pop_back();
        }

        std::cout << "Received: " << message << "\n";

        if (message == "CLIENTS") {
            std::lock_guard<std::mutex> lock(client_mutex);
            std::string response{"CLIENTS " + std::to_string(client_count) + "\n"};
            ssize_t bytes_sent{send(client_fd, response.c_str(), response.size(), 0)};
            if (bytes_sent < 0) {
                std::cerr << "send failed: " << std::strerror(errno) << "\n";
                break;
            }
            continue;
        }

        if (message == "QUIT") {
            std::string response{"GOODBYE\n"};
            ssize_t bytes_sent{send(client_fd, response.c_str(), response.size(), 0)};
            if (bytes_sent < 0) {
                std::cerr << "send failed: " << std::strerror(errno) << "\n";
            }
            break;
        }

        ssize_t bytes_sent{send(client_fd, message.c_str(), message.size(), 0)};
        if (bytes_sent < 0) {
            std::cerr << "send failed: " << std::strerror(errno) << "\n";
            break;
        }
    }

    {
        std::lock_guard<std::mutex> lock(client_mutex);
        client_count--;
        std::cout << "Client disconnected. Total clients: " << client_count << "\n";
    }

    close_socket(client_fd);
}

int main() {
    try {
        // Initialize an Internet (AF_INET) socket using a reliable TCP connection (SOCK_STREAM).
        int server_fd{socket(AF_INET, SOCK_STREAM, 0)};
        if (server_fd < 0) {
            fail("socket failed");
        }

        int reuse{1};
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            close_socket(server_fd);
            fail("setsockopt failed");
        }

        // Define the address at which the socket will accept connection requests.
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET; // Internet connections only.
        server_addr.sin_addr.s_addr = INADDR_ANY; // Any networking interface on the computer (Wi-Fi, ethernet, etc.)
        server_addr.sin_port = htons(PORT); // Convert the port number from local-machine endianness to Network (big) endianness.

        // Bind our socket to the desired address.
        if (bind(server_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            close_socket(server_fd);
            fail("bind failed");
        }

        // Wait for a connection request.
        if (listen(server_fd, BACKLOG) < 0) {
            close_socket(server_fd);
            fail("listen failed");
        }

        std::cout << "Server listening on port " << PORT << "...\n";

        int client_count{0};
        std::mutex client_mutex{};

        while (true) {
            sockaddr_in client_addr{};
            socklen_t client_len{sizeof(client_addr)};

            int client_fd{accept(
                server_fd,
                reinterpret_cast<sockaddr*>(&client_addr),
                &client_len
            )};

            if (client_fd < 0) {
                std::cerr << "accept failed: " << std::strerror(errno) << "\n";
                continue;
            }

            char client_ip[INET_ADDRSTRLEN]{};
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

            std::cout << "Client connected from "
                      << client_ip << ":"
                      << ntohs(client_addr.sin_port)
                      << "\n";

            {
                std::lock_guard<std::mutex> lock(client_mutex);
                if (client_count >= MAX_CLIENTS) {
                    std::cout << "Max clients reached, rejecting connection.\n";
                    std::string error_msg{"ERROR too many clients\n"};
                    send(client_fd, error_msg.c_str(), error_msg.size(), 0);
                    close_socket(client_fd);
                    continue;
                }
            }

            std::thread client_thread(handleClient, client_fd, std::ref(client_count), std::ref(client_mutex));
            client_thread.detach();
        }

        close_socket(server_fd);
    }
    catch (const std::exception& ex) {
        std::cerr << "Server error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}