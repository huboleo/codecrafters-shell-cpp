#pragma once
#include <string>
#include <vector>

namespace completion {
std::vector<std::string> get_command_candidates(const std::vector<std::string>& builtins);
std::vector<std::string> get_file_candidates(const std::string& text);
} // namespace completion
