#include "completion/completion.hpp"
#include "parsing/command_parser.hpp"
#include "utils/fs_utils.hpp"
#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

std::vector<std::string>
completion::get_command_candidates(const std::vector<std::string>& builtins) {
    std::vector<std::string> candidates = builtins;

    for (const auto& dir : fs_utils::get_path_directories()) {
        std::error_code ec;
        std::filesystem::directory_iterator entries(dir, ec);

        if (ec) {
            continue;
        }

        for (const auto& entry : entries) {
            const auto path = entry.path();
            const auto name = path.filename().string();

            if (!name.empty() && fs_utils::is_executable(path.string())) {
                candidates.push_back(name);
            }
        }
    }

    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

    return candidates;
}

std::vector<std::string> completion::get_file_candidates(const std::string& text) {
    std::vector<std::string> candidates;

    auto slash_pos = text.find_last_of('/');

    std::string dir_part;
    std::string file_prefix;

    if (slash_pos == std::string::npos) {
        dir_part = "";
        file_prefix = text;
    } else {
        dir_part = text.substr(0, slash_pos + 1);
        file_prefix = text.substr(slash_pos + 1);
    }

    std::filesystem::path dir_to_scan = dir_part.empty() ? "." : dir_part;

    std::error_code ec;
    std::filesystem::directory_iterator entries(dir_to_scan, ec);

    if (ec) {
        return candidates;
    }

    for (const auto& entry : entries) {

        std::string name = entry.path().filename().string();

        if (!name.starts_with(file_prefix)) {
            continue;
        }

        std::error_code entry_ec;
        if (entry.is_directory(entry_ec)) {
            name += "/";
        }

        candidates.push_back(dir_part + name);
    }

    std::sort(candidates.begin(), candidates.end());

    return candidates;
}

std::optional<completion::CompleterContext> completion::get_completer_context(
    const std::string& line, int current_word_start, int current_word_end,
    const std::string& current_word,
    const std::vector<std::pair<std::string, std::string>>& registered_completions) {

    auto words_before_current = command_parser::split_command(line.substr(0, current_word_start));

    if (words_before_current.empty()) {
        return std::nullopt;
    }

    const auto& cmd = words_before_current[0];

    auto it = std::find_if(registered_completions.begin(), registered_completions.end(),
                           [&](const auto& pair) { return pair.first == cmd; });

    if (it == registered_completions.end()) {
        return std::nullopt;
    }

    std::string previous_word = words_before_current.size() >= 2 ? words_before_current.back() : "";

    return completion::CompleterContext{
        .script_path = it->second,
        .command = cmd,
        .current_word = current_word,
        .previous_word = previous_word,
        .comp_line = line,
        .comp_point = current_word_end,
    };
}
