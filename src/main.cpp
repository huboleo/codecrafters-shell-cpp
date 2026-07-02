#include <cstdlib>
#include <cwctype>
#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <vector>

std::string ltrim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) {
        return "";
    }

    return str.substr(start);
}

bool is_executable(const std::string& str) { return access(str.c_str(), X_OK) == 0; }

std::vector<std::string> get_path_directories() {
    auto dirs = std::vector<std::string>();
    const char* path = std::getenv("PATH");
    if (path == nullptr) {
        return dirs;
    }

    std::stringstream ss(path);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        dirs.push_back(dir);
    }

    return dirs;
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
                auto dirs = get_path_directories();
                bool found = false;
                for (auto& dir : dirs) {
                    std::string candidate = dir + "/" + rest;
                    if (is_executable(candidate)) {
                        std::println("{} is {}", rest, candidate);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::println("{}: not found", rest);
                }
            }
        } else {
            std::println("{}: command not found", cmd);
        }
    }
}
