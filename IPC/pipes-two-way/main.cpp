// ipc_two_pipes.cpp
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    int parentToChild[2]{};
    int childToParent[2]{};

    if (pipe(parentToChild) == -1 || pipe(childToParent) == -1) {
        std::cerr << "pipe failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    pid_t pid{fork()};

    if (pid == -1) {
        std::cerr << "fork failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    if (pid == 0) {
        // Child process.
        close(parentToChild[1]); // Child does not write to parentToChild.
        close(childToParent[0]); // Child does not read from childToParent.

        char buffer[128]{};
        ssize_t bytesRead{read(parentToChild[0], buffer, sizeof(buffer) - 1)};

        if (bytesRead == -1) {
            std::cerr << "child read failed: " << std::strerror(errno) << '\n';
            return 1;
        }

        std::cout << "Child received: " << buffer << '\n';

        std::string reply{"Hello parent. I received your message."};

        ssize_t bytesWritten{write(childToParent[1], reply.c_str(), reply.size())};

        if (bytesWritten == -1) {
            std::cerr << "child write failed: " << std::strerror(errno) << '\n';
            return 1;
        }

        close(parentToChild[0]);
        close(childToParent[1]);

        return 0;
    }

    // Parent process.
    close(parentToChild[0]); // Parent does not read from parentToChild.
    close(childToParent[1]); // Parent does not write to childToParent.

    std::string message{"Hello child. This is the parent."};

    ssize_t bytesWritten{write(parentToChild[1], message.c_str(), message.size())};

    if (bytesWritten == -1) {
        std::cerr << "parent write failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    close(parentToChild[1]); // Done sending.

    char buffer[128]{};
    ssize_t bytesRead{read(childToParent[0], buffer, sizeof(buffer) - 1)};

    if (bytesRead == -1) {
        std::cerr << "parent read failed: " << std::strerror(errno) << '\n';
        return 1;
    }

    std::cout << "Parent received: " << buffer << '\n';

    close(childToParent[0]);

    int status{};
    waitpid(pid, &status, 0);

    return 0;
}