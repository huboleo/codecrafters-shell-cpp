#include "process.hpp"
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void process::run_executable(const std::string& path, const std::vector<std::string>& args) {
    std::vector<char*> argv;
    for (const auto& arg : args) {
        // Cast is ok here cause execv does not modify data in argv
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

std::vector<std::string> process::run_and_capture_lines(const std::string& path) {

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

        char* argv[] = {
            // Cast is ok here cause execv does not modify data in argv
            const_cast<char*>(path.c_str()),
            nullptr,
        };

        execv(path.c_str(), argv);
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
