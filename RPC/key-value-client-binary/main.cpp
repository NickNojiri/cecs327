#include "RemoteKeyValueStore.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void printMenu() {
    std::cout << "\n" << std::string(60, '=') << '\n';
    std::cout << "  Remote Key-Value Store Menu\n";
    std::cout << std::string(60, '=') << '\n';
    std::cout << "  1) Add pairs to the store\n";
    std::cout << "  2) Print store values\n";
    std::cout << "  0) Exit\n";
    std::cout << std::string(60, '=') << '\n';
}

// Option 1: read "key value" lines from the user, PUT each pair into the store,
// then print all of the keys currently in the store.
void optionAddPairs(RemoteKeyValueStore& store) {
    std::cout << "\n--- Option 1: Add Pairs to the Store ---\n";
    std::cout << "Enter \"key value\" pairs, one per line (empty line to finish):\n";

    std::string line;
    int added = 0;

    while (true) {
        std::cout << "  > ";
        if (!std::getline(std::cin, line) || line.empty()) {
            break;
        }

        // Split the line on whitespace: first token is the key, second is the value.
        std::istringstream input{line};
        std::string key;
        std::string value;
        if (!(input >> key >> value)) {
            std::cout << "    Skipped (need a key and a value): " << line << '\n';
            continue;
        }

        if (store.put(key, value)) {
            ++added;
            std::cout << "    Put: " << key << " = " << value << '\n';
        } else {
            std::cout << "    Failed to put: " << key << '\n';
        }
    }

    std::cout << "\nAdded " << added << " pair(s).\n";

    std::cout << "\nKeys currently in the store:\n";
    auto allKeys = store.keys();
    if (!allKeys.has_value()) {
        std::cout << "  [Error getting keys]\n";
        return;
    }
    if (allKeys->empty()) {
        std::cout << "  [No keys]\n";
        return;
    }
    for (const std::string& key : allKeys.value()) {
        std::cout << "  - " << key << '\n';
    }
}

// Option 2: print all of the store's values in alphabetical order. There is no
// "values" operation on the server, so we ask for the keys, GET each value, build
// a list, sort it, and print it.
void optionPrintValues(RemoteKeyValueStore& store) {
    std::cout << "\n--- Option 2: Print Store Values ---\n";

    auto allKeys = store.keys();
    if (!allKeys.has_value()) {
        std::cout << "Error: could not get keys from the store.\n";
        return;
    }
    if (allKeys->empty()) {
        std::cout << "The store is empty.\n";
        return;
    }

    std::vector<std::string> values;
    values.reserve(allKeys->size());
    for (const std::string& key : allKeys.value()) {
        auto value = store.get(key);
        if (value.has_value()) {
            values.push_back(value.value());
        } else {
            std::cout << "  [Warning] no value for key: " << key << '\n';
        }
    }

    std::sort(values.begin(), values.end());

    std::cout << "\nValues in alphabetical order:\n";
    for (const std::string& value : values) {
        std::cout << "  " << value << '\n';
    }
}

int main() {
    try {
        std::cout << "\n" << std::string(60, '=') << '\n';
        std::cout << "  Remote Key-Value Store RPC Client\n";
        std::cout << std::string(60, '=') << '\n';
        std::cout << "\nConnecting to server at 127.0.0.1:9090...\n";

        RemoteKeyValueStore store("127.0.0.1", 9090);
        std::cout << "Connected!\n";

        int choice = -1;
        while (choice != 0) {
            printMenu();
            std::cout << "Enter choice (0-2): ";
            if (!(std::cin >> choice)) {
                break;
            }
            std::cin.ignore();

            switch (choice) {
                case 0:
                    std::cout << "\nGoodbye!\n\n";
                    break;
                case 1:
                    optionAddPairs(store);
                    break;
                case 2:
                    optionPrintValues(store);
                    break;
                default:
                    std::cout << "\nInvalid choice. Please enter 0, 1, or 2.\n";
                    break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
