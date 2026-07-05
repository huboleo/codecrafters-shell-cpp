#pragma once

#include <optional>
#include <string>
#include <vector>

namespace fs_utils {

std::vector<std::string> get_path_directories();
bool is_executable(const std::string& path);
std::optional<std::string> find_executable(const std::string& command);
bool cd(const std::string& path);
std::optional<std::string> resolve_home_directory();
std::vector<std::string> get_files_in_current_directory();

namespace executables {
void run_executable(const std::string& path, const std::vector<std::string>& args);
}
} // namespace fs_utils
