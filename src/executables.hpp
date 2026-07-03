#pragma once
#include <cstdlib>
#include <cwctype>
#include <optional>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace executables {
inline std::vector<std::string> get_path_directories() {
    auto dirs = std::vector<std::string>();
    const char* path = std::getenv("PATH");
    if (path == nullptr) {
        return dirs;
    }

    std::stringstream ss(path);
    std::string dir;

    while (std::getline(ss, dir, ':')) {
        dirs.push_back(dir);
    }

    return dirs;
}

inline std::optional<std::string> find_executable(const std::string& command) {
    for (const auto& dir : get_path_directories()) {
        std::string candidate = dir + "/" + command;

        if (access(candidate.c_str(), X_OK) == 0) {
            return candidate;
        }
    }

    return std::nullopt;
}

inline void run_executable(const std::string& path, const std::vector<std::string>& args) {
    std::vector<char*> argv;
    for (const auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }

    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid == 0) {
        execv(path.c_str(), argv.data());
        std::exit(1);
    }

    waitpid(pid, nullptr, 0);
}

} // namespace executables
