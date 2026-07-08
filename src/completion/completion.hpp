#pragma once
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace completion {
struct CompleterContext {
    std::string script_path;
    std::string command;
    std::string current_word;
    std::string previous_word;
};
std::vector<std::string> get_command_candidates(const std::vector<std::string>& builtins);
std::vector<std::string> get_file_candidates(const std::string& text);
std::optional<CompleterContext> get_completer_context(
    const std::string& line, int current_word_start, const std::string& current_word,
    const std::vector<std::pair<std::string, std::string>>& registered_completions);
} // namespace completion
