#include "command_parser.hpp"
#include "fs_utils.hpp"
#include "string_utils.hpp"
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <unistd.h>
#include <unordered_set>

int main() {

    const auto shell_builtin_commands =
        std::unordered_set<std::string>{"exit", "echo", "type", "pwd"};

    while (true) {
        std::print("$ ");
        std::fflush(stdout);

        std::string input;
        std::getline(std::cin, input);

        auto trimmed_input = string_utils::ltrim(input);

        if (trimmed_input.empty()) {
            continue;
        }

        auto parts = command_parser::split_command(trimmed_input);

        if (parts.empty()) {
            continue;
        }

        auto parsed_command = command_parser::parse_command(parts);
        auto& command_parts = parsed_command.args;

        if (command_parts.empty()) {
            continue;
        }

        auto cmd = command_parts.at(0);

        if (cmd == "exit") {
            break;
        }

        int saved_stdout = -1;

        if (parsed_command.stdout_redirect.has_value()) {
            std::fflush(stdout);

            saved_stdout = dup(STDOUT_FILENO);

            if (saved_stdout == -1) {
                std::println(stderr, "redirection error");
                continue;
            }

            int fd = open(parsed_command.stdout_redirect.value().c_str(),
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd == -1) {
                close(saved_stdout);
                std::println(stderr, "redirection error");
                continue;
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
                close(fd);
                close(saved_stdout);
                std::println(stderr, "redirection error");
                continue;
            }
            close(fd);
        }

        if (cmd == "echo") {
            for (size_t i = 1; i < command_parts.size(); ++i) {
                if (i > 1) {
                    std::print(" ");
                }
                std::print("{}", command_parts[i]);
            }
            std::println();
        } else if (cmd == "type") {
            if (command_parts.size() >= 2) {
                auto program_name = command_parts.at(1);
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
            }
        } else if (cmd == "pwd") {
            std::println("{}", std::filesystem::current_path().string());
        } else if (cmd == "cd") {
            if (command_parts.size() >= 2) {
                auto requested_path = command_parts[1];
                if (requested_path.starts_with("~") || requested_path.starts_with("~/")) {
                    auto home_dir = fs_utils::resolve_home_directory();
                    if (home_dir.has_value()) {
                        auto resolved_path = home_dir.value() + requested_path.substr(1);
                        if (!fs_utils::cd(resolved_path)) {
                            std::println("cd: {}: No such file or directory", command_parts.at(1));
                        }
                    }
                } else {
                    if (!fs_utils::cd(command_parts.at(1))) {
                        std::println("cd: {}: No such file or directory", command_parts.at(1));
                    }
                }
            }

        } else {
            auto program = fs_utils::find_executable(cmd);
            if (program.has_value()) {
                fs_utils::executables::run_executable(program.value(), command_parts);
            } else {
                std::println("{}: command not found", cmd);
            }
        }

        if (saved_stdout != -1) {
            std::fflush(stdout);
            if (dup2(saved_stdout, STDOUT_FILENO) == -1) {
                close(saved_stdout);
                std::println(stderr, "failed to restore stdout");
            } else {
                close(saved_stdout);
            }
        }
    }
}
