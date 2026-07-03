#include "executables.hpp"
#include "string_utils.hpp"
#include <filesystem>
#include <iostream>
#include <print>
#include <string>
#include <unordered_set>
#include <vector>

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

        auto parts = string_utils::split(trimmed_input);

        auto cmd = parts.at(0);

        if (cmd == "exit") {
            break;
        }

        if (cmd == "echo") {
            auto rest = string_utils::ltrim(trimmed_input.substr(trimmed_input.find("echo") + 4));
            std::println("{}", rest);
        } else if (cmd == "type") {
            auto program_name = parts.at(1);
            if (shell_builtin_commands.contains(program_name)) {
                std::println("{} is a shell builtin", program_name);
            } else {
                auto path = executables::find_executable(program_name);
                if (path.has_value()) {
                    std::println("{} is {}", program_name, path.value());
                } else {
                    std::println("{}: not found", program_name);
                }
            }
        } else if (cmd == "pwd") {
            std::println("{}", std::filesystem::current_path().string());
        } else {
            auto program = executables::find_executable(cmd);
            if (program.has_value()) {
                executables::run_executable(program.value(), parts);
            } else {
                std::println("{}: command not found", cmd);
            }
        }
    }
}
