#include "completion.hpp"
#include "fs_utils.hpp"
#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>
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
}
