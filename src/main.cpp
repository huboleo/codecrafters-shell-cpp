#include "builtins/builtins.hpp"
#include "completion/completion.hpp"
#include "history/history_manager.hpp"
#include "jobs/job_table.hpp"
#include "parsing/command_parser.hpp"
#include "process/process.hpp"
#include "redirection/output_redirect.hpp"
#include "redirection/redirection_types.hpp"
#include "shell/shell_context.hpp"
#include "utils/fs_utils.hpp"
#include "utils/string_utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <optional>
#include <print>
#include <readline/history.h>
#include <readline/readline.h>
#include <string>
#include <unistd.h>
#include <vector>

std::vector<std::pair<std::string, std::string>> registered_completions;
std::optional<completion::CompleterContext> current_completer_context;

std::optional<std::string> get_env_variable(const std::string& name) {
    const char* value = std::getenv(name.c_str());

    if (value == nullptr) {
        return std::nullopt;
    }

    return std::string(value);
}

char* command_generator(const char* text, int state) {
    static size_t index;
    static std::string prefix;
    static std::vector<std::string> candidates;

    if (state == 0) {
        index = 0;
        prefix = text;
        candidates = completion::get_command_candidates(builtins::names());
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

    JobTable job_table;
    HistoryManager history_manager;

    auto history_file_path = get_env_variable("HISTFILE");
    if (history_file_path.has_value()) {
        history_manager.set_file_path(history_file_path.value());
        history_manager.load_from_file();
    }

    ShellContext shell_context = {
        .job_table = job_table,
        .history_manager = history_manager,
        .registered_completions = registered_completions,
    };

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

        add_history(trimmed_input.c_str());

        auto parts = command_parser::split_command(trimmed_input);

        if (parts.empty()) {
            continue;
        }

        command_parser::ParsedLine parsed_line = command_parser::parse_line(parts);

        if (parsed_line.parse_error.has_value()) {
            std::println("{}",
                         command_parser::parse_error_message(parsed_line.parse_error.value()));
            continue;
        }

        if (parsed_line.commands.empty()) {
            continue;
        }

        if (parsed_line.commands.size() > 1) {
            auto result = process::run_pipeline(parsed_line.commands, shell_context,
                                                parsed_line.should_run_in_background);

            if (parsed_line.should_run_in_background && result.background_pid.has_value()) {
                pid_t pid = result.background_pid.value();
                int job_id = job_table.add(pid, trimmed_input);
                std::println("[{}] {}", job_id, pid);
            }

            continue;
        }

        const auto& command = parsed_line.commands[0];

        OutputRedirect redirect;

        if (command.stdout_redirect.has_value()) {
            if (!redirect.redirect_stdout(command.stdout_redirect->path,
                                          command.stdout_redirect->mode)) {
                std::println(stderr, "redirection error");
                continue;
            }
        }

        if (command.stderr_redirect.has_value()) {
            if (!redirect.redirect_stderr(command.stderr_redirect->path,
                                          command.stderr_redirect->mode)) {
                std::println(stderr, "redirection error");
                continue;
            }
        }

        const auto& command_parts = command.args;

        if (command_parts.empty()) {
            continue;
        }

        const auto& cmd = command_parts[0];

        if (builtins::is_builtin(cmd)) {
            builtins::run(command_parts, shell_context);

            if (shell_context.should_exit) {
                break;
            }
        } else {
            auto program = fs_utils::find_executable(cmd);
            if (program.has_value()) {
                if (parsed_line.should_run_in_background) {
                    auto result = process::run_background_job(program.value(), command_parts);

                    if (result.has_value()) {
                        pid_t pid = result.value();
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
