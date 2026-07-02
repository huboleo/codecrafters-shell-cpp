#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

std::vector<std::string> split(const std::string& input) {
    std::stringstream ss(input);
    std::string word;
    std::vector<std::string> parts;

    while (ss >> word) {
        parts.push_back(word);
    }

    return parts;
}

int main() {

    const auto shell_builtin_commands = std::unordered_set<std::string>{"exit", "echo"};

    while (true) {
        std::print("$ ");
        std::string input;
        std::getline(std::cin, input);

        const auto parts = split(input);

        if (parts.empty()) {
            continue;
        }

        const auto cmd = parts.at(0);

        if (cmd == "exit") {
            break;
        } else if (cmd == "type") {
            if (shell_builtin_commands.contains(parts.at(1))) {
                std::println("{} is a shell builtin", parts.at(1));
            } else {
                std::println("{} invalid_command", parts.at(1));
            }
        } else if (input.substr(0, 5) == "echo ") {
            std::println("{}", input.substr(5));
            continue;
        } else {
            std::println("{}: command not found", input);
        }
    }
}
