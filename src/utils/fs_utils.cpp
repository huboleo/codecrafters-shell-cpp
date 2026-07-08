#include "utils/fs_utils.hpp"
#include <cstdlib>
#include <cwctype>
#include <optional>
#include <sstream>
#include <string>
#include <sys/wait.h>
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
