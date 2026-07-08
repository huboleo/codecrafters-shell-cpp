#pragma once
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace completion {
std::vector<std::string> get_command_candidates(const std::vector<std::string>& builtins);
std::vector<std::string> get_file_candidates(const std::string& text);
std::optional<std::string> get_registered_completer_for_line(
    const std::string& line,
    const std::vector<std::pair<std::string, std::string>>& registered_completions);
} // namespace completion
