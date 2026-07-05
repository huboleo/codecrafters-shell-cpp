#include "fs_utils.hpp"
#include <algorithm>
#include <cstdlib>
#include <cwctype>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>
#include <vector>

std::vector<std::string> fs_utils::get_path_directories() {
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

bool fs_utils::is_executable(const std::string& path) { return access(path.c_str(), X_OK) == 0; }

std::optional<std::string> fs_utils::find_executable(const std::string& command) {
    for (const auto& dir : get_path_directories()) {
        std::string candidate = dir + "/" + command;

        if (is_executable(candidate)) {
            return candidate;
        }
    }

    return std::nullopt;
}

bool fs_utils::cd(const std::string& path) { return chdir(path.c_str()) == 0; }

std::optional<std::string> fs_utils::resolve_home_directory() {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
        return std::nullopt;
    }
    return std::string(home);
}

std::vector<std::string> fs_utils::get_files_in_current_directory() {
    std::vector<std::string> files;

    std::error_code ec;
    auto current_dir = std::filesystem::current_path(ec);

    if (ec) {
        return files;
    }

    std::filesystem::directory_iterator entries(current_dir, ec);

    if (ec) {
        return files;
    }

    for (const auto& entry : entries) {
        auto name = entry.path().filename().string();

        if (name.empty()) {
            continue;
        }

        std::error_code entry_ec;

        if (entry.is_directory(entry_ec)) {
            name += "/";
        }

        files.push_back(name);
    }

    std::sort(files.begin(), files.end());

    return files;
}

void fs_utils::executables::run_executable(const std::string& path,
                                           const std::vector<std::string>& args) {
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
