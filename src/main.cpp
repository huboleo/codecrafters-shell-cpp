#include "fs_utils.hpp"
#include "string_utils.hpp"
#include <filesystem>
#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <unordered_set>

int main() {

    const auto shell_builtin_commands =
        std::unordered_set<std::string>{"exit", "echo", "type", "pwd"};

    while (true) {
        std::print("$ ");
        std::string input;
        std::getline(std::cin, input);

        auto trimmed_input = string_utils::ltrim(input);

        if (trimmed_input.empty()) {
            continue;
        }

        auto whitespace_pos = input.find_first_of(" \t\n\r\f\v");

        auto parts = string_utils::split_command(trimmed_input);

        auto cmd = parts.at(0);

        if (cmd == "exit") {
            break;
        }

        if (cmd == "echo") {
            if (parts.size() < 2) {
                std::println();
                continue;
            }
            std::println("{}", parts.at(1));
        } else if (cmd == "type") {
            auto program_name = parts.at(1);
            if (shell_builtin_commands.contains(program_name)) {
                std::println("{} is a shell builtin", program_name);
            } else {
                auto path = fs_utils::find_executable(program_name);
                if (path.has_value()) {
                    std::println("{} is {}", program_name, path.value());
                } else {
                    std::println("{}: not found", program_name);
                }
            }
        } else if (cmd == "pwd") {
            std::println("{}", std::filesystem::current_path().string());
        } else if (cmd == "cd") {
            if (parts.size() < 2) {
                continue;
            }
            if (parts.at(1).starts_with("~")) {
                auto home_dir = fs_utils::resolve_home_directory();
                if (home_dir.has_value()) {
                    fs_utils::cd(home_dir.value());
                }
            } else {
                if (!fs_utils::cd(parts.at(1))) {
                    std::println("cd: {}: No such file or directory", parts.at(1));
                }
            }

        } else {
            auto program = fs_utils::find_executable(cmd);
            if (program.has_value()) {
                fs_utils::executables::run_executable(program.value(), parts);
            } else {
                std::println("{}: command not found", cmd);
            }
        }
    }
}
