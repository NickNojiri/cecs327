#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace {
constexpr int PORT{9090};
constexpr int BACKLOG{5};
constexpr int BUFFER_SIZE{1024};

void close_socket(int fd) {
  if (fd >= 0) {
    close(fd);
  }
}

void fail(const std::string &message) {
  throw std::runtime_error{message + ": " + std::strerror(errno)};
}

// Returns the current local time formatted as hh:mm:ss (ISO 8601 time).
std::string get_current_time() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_t = std::chrono::system_clock::to_time_t(now);
  std::tm local_tm{};
#ifdef _WIN32
  localtime_s(&local_tm, &now_t);
#else
  localtime_r(&now_t, &local_tm);
#endif
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << local_tm.tm_hour << ":"
      << std::setw(2) << local_tm.tm_min << ":" << std::setw(2)
      << local_tm.tm_sec;
  return oss.str();
}

std::string build_response(const std::string &message, bool &should_quit, int active_clients) {
  should_quit = false;

  // --- ECHO: message starts with "ECHO" ---
  if (message.substr(0, 4) == "ECHO") {
    return message.substr(4) + "\n";
  }

  // --- TIME: message starts with "TIME" ---
  if (message.substr(0, 4) == "TIME") {
    return get_current_time() + "\n";
  }

  // Trim trailing whitespace/newlines for command matching
  std::string trimmed = message;
  while (!trimmed.empty() &&
         (trimmed.back() == '\n' || trimmed.back() == '\r' ||
          trimmed.back() == ' ')) {
    trimmed.pop_back();
  }

  // --- CLIENTS ---
  if (trimmed == "CLIENTS") {
    return std::to_string(active_clients) + "\n";
  }

  // --- HELP ---
  if (trimmed == "HELP") {
    return std::string{
        "Supported commands:\n"
        "  ECHO <message>  - Echoes back <message>\n"
        "  TIME            - Returns the current server time (hh:mm:ss)\n"
        "  CLIENTS         - Returns the count of active clients\n"
        "  HELP            - Shows this help message\n"
        "  QUIT            - Disconnects from the server\n"};
  }

  // --- QUIT ---
  if (trimmed == "QUIT") {
    should_quit = true;
    return "GOODBYE\n";
  }

  // Otherwise, return error unknown command
  return "ERROR unknown command\n";
}


void handleClient(int client_fd, int &active_clients, std::mutex &active_clients_mutex) {
  // Lock the mutex first to check active client count safely
  {
    std::lock_guard<std::mutex> lock{active_clients_mutex};
    if (active_clients >= 3) {
      std::string response = "ERROR too many clients\n";
      send(client_fd, response.c_str(), response.size(), 0);
      shutdown(client_fd, SHUT_WR);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      close_socket(client_fd);
      return;
    }
    active_clients++;
  }

  char buffer[BUFFER_SIZE]{};

  while (true) {
    // Receive up to BUFFER_SIZE-1 bytes from the client.
    ssize_t bytes_received{recv(client_fd, buffer, BUFFER_SIZE - 1, 0)};

    if (bytes_received <= 0) {
      break;
    }

    buffer[bytes_received] = '\0';
    std::string message{buffer};

    // Print the received message to the server console.
    std::cout << "Received: " << message;

    // Read current client count safely to pass to build_response
    int current_clients = 0;
    {
      std::lock_guard<std::mutex> lock{active_clients_mutex};
      current_clients = active_clients;
    }

    // Determine the appropriate response.
    bool should_quit{false};
    std::string response{build_response(message, should_quit, current_clients)};

    // Send the response to the client.
    ssize_t bytes_sent{send(client_fd, response.c_str(), response.size(), 0)};

    if (bytes_sent < 0 || should_quit) {
      break;
    }
  }

  // Clean up: decrement counter under mutex lock and close socket
  {
    std::lock_guard<std::mutex> lock{active_clients_mutex};
    active_clients--;
  }
  close_socket(client_fd);
  std::cout << "Client disconnected.\n";
}
} // namespace

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
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind our socket to the desired address.
    if (bind(server_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
      close_socket(server_fd);
      fail("bind failed");
    }

    // Wait for a connection request.
    if (listen(server_fd, BACKLOG) < 0) {
      close_socket(server_fd);
      fail("listen failed");
    }

    int active_clients = 0;
    std::mutex active_clients_mutex;

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
      // Initialize a second socket to use when responding to a client.
      sockaddr_in client_addr{};
      socklen_t client_len{sizeof(client_addr)};

      // Accept the incoming request.
      int client_fd{accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len)};

      if (client_fd < 0) {
        std::cerr << "accept failed: " << std::strerror(errno) << "\n";
        continue;
      }

      char client_ip[INET_ADDRSTRLEN]{};
      inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

      std::cout << "Client connected from " << client_ip << ":" << ntohs(client_addr.sin_port) << "\n";

      // Spawn a thread to handle client interaction
      std::thread client_thread{handleClient, client_fd, std::ref(active_clients), std::ref(active_clients_mutex)};
      client_thread.detach();
    }

    close_socket(server_fd);
  }
  catch (const std::exception &ex) {
    std::cerr << "Server error: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}