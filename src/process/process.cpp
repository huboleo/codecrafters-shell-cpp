#include "process.hpp"
#include <optional>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

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
