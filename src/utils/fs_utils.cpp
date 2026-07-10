#include "utils/fs_utils.hpp"
#include <cstdlib>
#include <cwctype>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using fs_utils::WriteMode;

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

std::vector<std::string> fs_utils::read_lines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream file(path);

    if (!file) {
        return lines;
    }

    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}

bool fs_utils::write_str_vector_to_file(const std::string& path,
                                        const std::vector<std::string>& lines,
                                        WriteMode write_mode) {

    auto open_mode = std::ios::out;

    if (write_mode == WriteMode::Append) {
        open_mode |= std::ios::app;
    }

    std::ofstream file(path, open_mode);

    if (!file) {
        return false;
    }

    for (const auto& line : lines) {
        file << line << '\n';
    }

    file << '\n';

    return static_cast<bool>(file);
}
