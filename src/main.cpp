#include "command_parser.hpp"
#include "completion.hpp"
#include "fs_utils.hpp"
#include "output_redirect.hpp"
#include "redirection_types.hpp"
#include "string_utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <optional>
#include <print>
#include <readline/readline.h>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <vector>

const std::vector<std::string> shell_builtin_commands = {
    "cd", "echo", "exit", "pwd", "type",
};

char* command_generator(const char* text, int state) {
    static size_t index;
    static std::string prefix;
    static std::vector<std::string> candidates;

    if (state == 0) {
        index = 0;
        prefix = text;
        candidates = completion::get_command_candidates(shell_builtin_commands);
    }

    while (index < candidates.size()) {
        const auto& candidate = candidates[index++];

        if (candidate.starts_with(prefix)) {
            return strdup(candidate.c_str());
        }
    }

    return nullptr;
}

char* file_generator(const char* text, int state) {
    static size_t index;
    static std::vector<std::string> candidates;

    if (state == 0) {
        index = 0;
        candidates = completion::get_file_candidates(text);
    }

    while (index < candidates.size()) {
        const auto& candidate = candidates[index++];
        if (candidate.ends_with("/")) {
            rl_completion_append_character = '\0';
        } else {
            rl_completion_append_character = ' ';
        }

        return strdup(candidate.c_str());
    }

    return nullptr;
}

char** complete_command(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    return rl_completion_matches(text, file_generator);
}

int main() {
    rl_attempted_completion_function = complete_command;

    const std::unordered_set<std::string> shell_builtin_commands_set(shell_builtin_commands.begin(),
                                                                     shell_builtin_commands.end());

    while (true) {
        char* line = readline("$ ");

        if (line == nullptr) {
            break;
        }

        std::string input = line;
        free(line);

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

        OutputRedirect redirect;

        if (parsed_command.stdout_redirect.has_value()) {
            if (!redirect.redirect_stdout(parsed_command.stdout_redirect->path,
                                          parsed_command.stdout_redirect->mode)) {
                std::println(stderr, "redirection error");
                continue;
            }
        }

        if (parsed_command.stderr_redirect.has_value()) {
            if (!redirect.redirect_stderr(parsed_command.stderr_redirect->path,
                                          parsed_command.stderr_redirect->mode)) {
                std::println(stderr, "redirection error");
                continue;
            }
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
                if (shell_builtin_commands_set.contains(program_name)) {
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
    }
}
