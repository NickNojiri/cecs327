#include "RemoteList.h"
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
    for (const std::string& k : allKeys.value()) {
        std::cout << "  - " << k << '\n';
    }
}

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
    for (const std::string& k : allKeys.value()) {
        auto value = store.get(k);
        if (value.has_value()) {
            values.push_back(value.value());
        } else {
            std::cout << "  [Warning] no value for key: " << k << '\n';
        }
    }

    std::sort(values.begin(), values.end());

    std::cout << "\nValues in alphabetical order:\n";
    for (const std::string& v : values) {
        std::cout << "  " << v << '\n';
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
