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
} // namespace fs_utils
