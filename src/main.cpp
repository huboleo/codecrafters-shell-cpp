#include <cwctype>
#include <iostream>
#include <print>
#include <string>
#include <unordered_set>

std::string ltrim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) {
        return "";
    }

    return str.substr(start);
}

int main() {

    const auto shell_builtin_commands = std::unordered_set<std::string>{"exit", "echo", "type"};

    while (true) {
        std::print("$ ");
        std::string input;
        std::getline(std::cin, input);

        auto trimmed_cmd = ltrim(input);

        if (trimmed_cmd.empty()) {
            continue;
        }

        auto whitespace_pos = input.find_first_of(" \t\n\r\f\v");

        std::string cmd;
        std::string rest;

        if (whitespace_pos != std::string::npos) {
            cmd = trimmed_cmd.substr(0, whitespace_pos);
            rest = trimmed_cmd.substr(whitespace_pos + 1);
        } else {
            cmd = trimmed_cmd;
        }

        if (cmd == "exit") {
            break;
        }

        if (cmd == "echo") {
            std::println("{}", rest);
        } else if (cmd == "type") {
            if (shell_builtin_commands.contains(rest)) {
                std::println("{} is a shell builtin", rest);
            } else {
                std::println("{}: command not found", rest);
            }
        } else {
            std::println("{}: command not found", cmd);
        }
    }
}
