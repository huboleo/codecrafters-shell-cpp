#include "completion/completion.hpp"
#include "jobs/job_table.hpp"
#include "parsing/command_parser.hpp"
#include "process/process.hpp"
#include "redirection/output_redirect.hpp"
#include "redirection/redirection_types.hpp"
#include "utils/fs_utils.hpp"
#include "utils/string_utils.hpp"
#include <algorithm>
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
    "cd", "complete", "echo", "exit", "jobs", "pwd", "type",
};

std::vector<std::pair<std::string, std::string>> registered_completions;
std::optional<completion::CompleterContext> current_completer_context;

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

char* registered_completions_generator(const char* text, int state) {
    static size_t index;
    static std::vector<std::string> candidates;

    if (state == 0) {
        index = 0;

        // fine cause complete_command already fills the context;
        if (current_completer_context.has_value()) {
            const auto& context = current_completer_context.value();

            candidates = process::run_completer_script(context.script_path,
                                                       {
                                                           context.command,
                                                           context.current_word,
                                                           context.previous_word,
                                                       },
                                                       context.comp_line, context.comp_point);
        } else {
            candidates = {};
        }
    }

    while (index < candidates.size()) {
        const auto& candidate = candidates[index++];
        if (candidate.starts_with(text)) {
            rl_completion_append_character = ' ';
            return strdup(candidate.c_str());
        }
    }

    return nullptr;
}

char** complete_command(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }

    current_completer_context =
        completion::get_completer_context(rl_line_buffer, start, end, text, registered_completions);

    if (current_completer_context.has_value()) {
        return rl_completion_matches(text, registered_completions_generator);
    }

    return rl_completion_matches(text, file_generator);
}

int main() {
    rl_attempted_completion_function = complete_command;

    const std::unordered_set<std::string> shell_builtin_commands_set(shell_builtin_commands.begin(),
                                                                     shell_builtin_commands.end());
    JobTable job_table;

    while (true) {
        job_table.refresh();
        job_table.print_done();
        job_table.remove_done();

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
        } else if (cmd == "jobs") {
            job_table.refresh();
            job_table.print_all();
        } else if (cmd == "complete") {
            if (command_parts.size() >= 3) {
                if (command_parts[1] == "-p") {
                    auto it =
                        std::find_if(registered_completions.begin(), registered_completions.end(),
                                     [command_parts](const auto& item) {
                                         return item.first == command_parts[2];
                                     });
                    if (it != registered_completions.end()) {
                        auto formatted_command =
                            std::format("complete -C {}",
                                        string_utils::surround_with_single_quotes(it->second));
                        std::println("{} {}", formatted_command, it->first);
                    } else {
                        std::println("complete: {}: no completion specification", command_parts[2]);
                    }
                } else if (command_parts[1] == "-C") {
                    if (command_parts.size() >= 4) {
                        auto& key = command_parts[3];
                        std::erase_if(registered_completions,
                                      [&key](const auto& pair) { return pair.first == key; });
                        registered_completions.push_back({key, command_parts[2]});
                    }
                } else if (command_parts[1] == "-r") {
                    if (command_parts.size() >= 3) {
                        const auto& key = command_parts[2];
                        std::erase_if(registered_completions,
                                      [&key](const auto& pair) { return pair.first == key; });
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
                if (command_parts.size() >= 2 && command_parts.back() == "&") {
                    command_parts.pop_back();
                    auto result = process::run_background_job(program.value(), command_parts);
                    if (result.has_value()) {
                        auto pid = result.value();
                        int job_id = job_table.add(pid, trimmed_input);
                        std::println("[{}] {}", job_id, pid);
                    }
                } else {
                    process::run_executable(program.value(), command_parts);
                }
            } else {
                std::println("{}: command not found", cmd);
            }
        }
    }
}
