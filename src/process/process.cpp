#include "process.hpp"
#include "builtins/builtins.hpp"
#include "utils/fs_utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <optional>
#include <print>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
void close_if_open(int fd) {
    if (fd != -1) {
        close(fd);
    }
}

void terminate_started_processes(const std::vector<pid_t>& pids) {
    for (pid_t pid : pids) {
        kill(pid, SIGTERM);
    }

    for (pid_t pid : pids) {
        waitpid(pid, nullptr, 0);
    }
}

int wait_for_pipeline(const std::vector<pid_t>& pids) {
    int last_status = 1;

    for (size_t i = 0; i < pids.size(); ++i) {
        int status = 0;

        if (waitpid(pids[i], &status, 0) == -1) {
            continue;
        }

        if (i == pids.size() - 1) {
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                last_status = 128 + WTERMSIG(status);
            } else {
                last_status = 1;
            }
        }
    }

    return last_status;
}

bool redirect_fd_to_file(int target_fd, const RedirectTarget& target) {
    if (target_fd == STDOUT_FILENO) {
        std::fflush(stdout);
    } else if (target_fd == STDERR_FILENO) {
        std::fflush(stderr);
    }

    int flags = O_WRONLY | O_CREAT;

    if (target.mode == RedirectMode::Append) {
        flags |= O_APPEND;
    } else {
        flags |= O_TRUNC;
    }

    int file_fd = open(target.path.c_str(), flags, 0644);

    if (file_fd == -1) {
        return false;
    }

    if (dup2(file_fd, target_fd) == -1) {
        close(file_fd);
        return false;
    }

    close(file_fd);
    return true;
}

[[noreturn]] void run_pipeline_child_command(const command_parser::Command& command,
                                             ShellContext& shell_context) {
    if (command.stdout_redirect.has_value()) {
        if (!redirect_fd_to_file(STDOUT_FILENO, command.stdout_redirect.value())) {
            std::println(stderr, "redirection error");
            std::fflush(stderr);
            std::_Exit(1);
        }
    }

    if (command.stderr_redirect.has_value()) {
        if (!redirect_fd_to_file(STDERR_FILENO, command.stderr_redirect.value())) {
            std::println(stderr, "redirection error");
            std::fflush(stderr);
            std::_Exit(1);
        }
    }

    const auto& command_args = command.args;

    if (command_args.empty()) {
        std::_Exit(0);
    }

    const auto& cmd = command_args[0];

    if (builtins::is_builtin(cmd)) {
        int status = builtins::run(command_args, shell_context);
        std::fflush(stdout);
        std::fflush(stderr);
        std::_Exit(status);
    }

    auto program = fs_utils::find_executable(cmd);

    if (!program.has_value()) {
        std::println(stderr, "{}: command not found", cmd);
        std::fflush(stderr);
        std::_Exit(1);
    }

    std::vector<std::string> args_str = command_args;
    std::vector<char*> argv;
    argv.reserve(args_str.size() + 1);

    for (auto& arg : args_str) {
        argv.push_back(arg.data());
    }

    argv.push_back(nullptr);

    execv(program->c_str(), argv.data());
    std::_Exit(1);
}
} // namespace

void process::run_executable(const std::string& path, const std::vector<std::string>& args) {
    std::vector<char*> argv;
    for (const auto& arg : args) {
        // Cast should be ok here cause execv does not modify data in argv
        argv.push_back(const_cast<char*>(arg.c_str()));
    }

    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        execv(path.c_str(), argv.data());
        std::_Exit(1);
    }

    waitpid(pid, nullptr, 0);
}

std::vector<std::string> process::run_completer_script(const std::string& path,
                                                       const std::vector<std::string>& args,
                                                       const std::string& comp_line,
                                                       int comp_point) {
    std::vector<std::string> argv_str = {path};
    argv_str.insert(argv_str.end(), args.begin(), args.end());

    std::vector<char*> argv;
    argv.reserve(argv_str.size() + 1);

    for (auto& arg : argv_str) {
        argv.push_back(arg.data());
    }

    argv.push_back(nullptr);

    int fildes[2];

    if (pipe(fildes) == -1) {
        return {};
    }

    pid_t pid = fork();

    if (pid == -1) {
        close(fildes[0]);
        close(fildes[1]);
        return {};
    }

    if (pid == 0) {
        close(fildes[0]);
        dup2(fildes[1], STDOUT_FILENO);
        close(fildes[1]);

        setenv("COMP_LINE", comp_line.c_str(), 1);
        setenv("COMP_POINT", std::to_string(comp_point).c_str(), 1);

        execv(path.c_str(), argv.data());
        std::_Exit(1);
    }

    close(fildes[1]);

    std::string output;
    char buffer[256];

    ssize_t bytes_read;

    while ((bytes_read = read(fildes[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, bytes_read);
    }

    close(fildes[0]);
    waitpid(pid, nullptr, 0);

    std::vector<std::string> candidates;
    std::stringstream ss(output);
    std::string line;

    while (std::getline(ss, line)) {
        if (!line.empty()) {
            candidates.push_back(line);
        }
    }

    return candidates;
}

std::optional<pid_t> process::run_background_job(const std::string& path,
                                                 const std::vector<std::string>& args) {

    std::vector<std::string> args_str = args;
    std::vector<char*> argv;

    for (auto& arg : args_str) {
        argv.push_back(arg.data());
    }

    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        execv(path.c_str(), argv.data());
        std::_Exit(1);
    }

    if (pid > 0) {
        return pid;
    }

    return std::nullopt;
}

process::PipelineResult process::run_pipeline(const std::vector<command_parser::Command>& commands,
                                              ShellContext& shell_context, bool run_in_background) {

    std::fflush(nullptr);

    process::PipelineResult result;
    std::vector<pid_t> pids;
    int previous_read_fd = -1;

    for (size_t i{0}; i < commands.size(); ++i) {
        bool is_last = i == commands.size() - 1;

        int pipefds[2] = {-1, -1};

        if (!is_last) {
            if (pipe(pipefds) == -1) {
                close_if_open(previous_read_fd);
                terminate_started_processes(pids);
                result.exit_status = 1;
                return result;
            }
        }

        pid_t pid = fork();

        if (pid == -1) {
            close_if_open(pipefds[0]);
            close_if_open(pipefds[1]);
            close_if_open(previous_read_fd);
            terminate_started_processes(pids);
            result.exit_status = 1;
            return result;
        }

        if (pid == 0) {
            if (previous_read_fd != -1 && dup2(previous_read_fd, STDIN_FILENO) == -1) {
                std::_Exit(1);
            }

            if (!is_last && dup2(pipefds[1], STDOUT_FILENO) == -1) {
                std::_Exit(1);
            }

            close_if_open(previous_read_fd);
            close_if_open(pipefds[0]);
            close_if_open(pipefds[1]);

            run_pipeline_child_command(commands[i], shell_context);
        }

        pids.push_back(pid);

        close_if_open(previous_read_fd);
        previous_read_fd = -1;

        if (!is_last) {
            close_if_open(pipefds[1]);
            previous_read_fd = pipefds[0];
        }
    }

    if (run_in_background) {
        if (!pids.empty()) {
            result.background_pid = pids.front();
        }

        result.exit_status = 0;
        return result;
    }

    result.exit_status = wait_for_pipeline(pids);
    return result;
}
