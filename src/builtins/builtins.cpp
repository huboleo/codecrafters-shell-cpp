#include "builtins/builtins.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <print>
#include <readline/history.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "shell/shell_context.hpp"
#include "utils/fs_utils.hpp"
#include "utils/string_utils.hpp"

namespace {
const std::vector<std::string> shell_builtins = {
    "cd", "complete", "declare", "echo", "exit", "history", "jobs", "pwd", "type",
};

const std::unordered_set<std::string> shell_builtins_set(shell_builtins.begin(),
                                                         shell_builtins.end());

int run_cd(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        std::println(stderr, "cd: too many arguments");
        return 2;
    }

    if (args.size() < 2) {
        return 2;
    }

    auto requested_path = args[1];

    if (requested_path == "~" || requested_path.starts_with("~/")) {
        auto home_dir = fs_utils::resolve_home_directory();

        if (!home_dir.has_value()) {
            std::println(stderr, "cd: HOME not set");
            return 1;
        }
        requested_path = home_dir.value() + requested_path.substr(1);
    }

    if (!fs_utils::cd(requested_path)) {
        std::println(stderr, "cd: {}: No such file or directory", args[1]);
        return 1;
    }

    return 0;
}

int run_echo(const std::vector<std::string>& args) {
    for (size_t i = 1; i < args.size(); ++i) {
        if (i > 1) {
            std::print(" ");
        }
        std::print("{}", args[i]);
    }
    std::println();
    return 0;
}

int run_complete(const std::vector<std::string>& args, ShellContext& shell_context) {
    if (args.size() < 3) {
        return 2;
    }

    auto flag = args[1];

    if (flag == "-p") {
        const auto& key = args[2];
        auto it = std::find_if(shell_context.registered_completions.begin(),
                               shell_context.registered_completions.end(),
                               [&key](const auto& pair) { return pair.first == key; });

        if (it == shell_context.registered_completions.end()) {
            std::println("complete: {}: no completion specification", key);
            return 1;
        }

        const auto formatted_command =
            std::format("complete -C {}", string_utils::surround_with_single_quotes(it->second));
        std::println("{} {}", formatted_command, it->first);
    } else if (flag == "-C") {
        if (args.size() < 4) {
            return 2;
        }

        const auto& key = args[3];
        std::erase_if(shell_context.registered_completions,
                      [&key](const auto& pair) { return pair.first == key; });

        shell_context.registered_completions.push_back({key, args[2]});
    } else if (flag == "-r") {
        const auto& key = args[2];
        std::erase_if(shell_context.registered_completions,
                      [&key](const auto& pair) { return pair.first == key; });
    } else {
        return 2;
    }

    return 0;
}

int run_jobs(ShellContext& shell_context) {
    shell_context.job_table.refresh();
    shell_context.job_table.print_all();
    shell_context.job_table.remove_done();
    return 0;
}

int run_exit(ShellContext& shell_context) {
    shell_context.should_exit = true;
    // shell_context.history_manager.append_to_file(const std::string& path)
    return 0;
}

int run_declare() { return 0; }

int run_pwd() {
    std::println("{}", std::filesystem::current_path().string());
    return 0;
}

int run_type(const std::vector<std::string>& args) {
    if (args.size() > 2) {
        std::println(stderr, "type: too many arguments");
        return 2;
    }

    if (args.size() < 2) {
        return 2;
    }

    auto program_name = args[1];

    if (shell_builtins_set.contains(program_name)) {
        std::println("{} is a shell builtin", program_name);
    } else {
        auto path = fs_utils::find_executable(program_name);
        if (!path.has_value()) {
            std::println("{}: not found", program_name);
            return 1;
        }

        std::println("{} is {}", program_name, path.value());
    }

    return 0;
}

} // namespace

bool builtins::is_builtin(const std::string& command) {
    return shell_builtins_set.contains(command);
}

int builtins::run(const std::vector<std::string>& args, ShellContext& shell_context) {

    if (args.empty()) {
        return 0;
    }

    auto command = args[0];

    if (command == "cd") {
        return run_cd(args);
    } else if (command == "complete") {
        return run_complete(args, shell_context);
    } else if (command == "declare") {

    } else if (command == "echo") {
        return run_echo(args);
    } else if (command == "exit") {
        return run_exit(shell_context);
    } else if (command == "history") {
        return shell_context.history_manager.run(args);
    } else if (command == "jobs") {
        return run_jobs(shell_context);
    } else if (command == "pwd") {
        return run_pwd();
    } else if (command == "type") {
        return run_type(args);
    } else {
        return 1;
    }
}

const std::vector<std::string>& builtins::names() { return shell_builtins; }
