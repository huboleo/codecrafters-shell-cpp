#pragma once

#include <optional>
#include <string>
#include <vector>

namespace fs_utils {

enum class WriteMode {
    Append,
    Trunc,
};

std::vector<std::string> get_path_directories();
bool is_executable(const std::string& path);
std::optional<std::string> find_executable(const std::string& command);
bool cd(const std::string& path);
std::optional<std::string> resolve_home_directory();
std::vector<std::string> read_lines(const std::string& path);
bool write_str_vector_to_file(const std::string& path, const std::vector<std::string>& lines,
                              WriteMode write_mode);
} // namespace fs_utils
