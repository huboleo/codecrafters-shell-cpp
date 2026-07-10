#include "history/history_manager.hpp"
#include "utils/fs_utils.hpp"

#include "utils/fs_utils.hpp"
#include "utils/string_utils.hpp"
#include <print>
#include <readline/history.h>
#include <string>
#include <unordered_map>
#include <vector>

using fs_utils::WriteMode;

void HistoryManager::set_file_path(const std::string& path) { file_path_ = path; }

bool HistoryManager::load_from_file() {
    if (file_path_.empty()) {
        return false;
    }

    auto lines = fs_utils::read_lines(file_path_);

    for (const auto& line : lines) {
        add_history(line.c_str());
    }

    append_offsets_[file_path_] = history_length;

    return true;
}

bool HistoryManager::append_to_file() {
    if (file_path_.empty()) {
        return false;
    }

    int offset = append_offsets_[file_path_];

    int first = history_base + offset;
    int last_exclusive = history_base + history_length;

    auto lines = HistoryManager::collect_lines(first, last_exclusive);

    bool write_result = fs_utils::write_str_vector_to_file(file_path_, lines, WriteMode::Append);

    if (!write_result) {
        return 1;
    }

    append_offsets_[file_path_] = history_length;

    return 0;
}

int HistoryManager::print_all() const {
    for (int i = history_base; i < history_base + history_length; ++i) {
        HIST_ENTRY* entry = history_get(i);

        if (entry != nullptr) {
            std::println("{} {}", i, entry->line);
        }
    }

    return 0;
}

int HistoryManager::print_last(int count) const {
    int last = history_base + history_length - 1;
    int first = std::max(history_base, last - count + 1);

    for (int i = first; i <= last; ++i) {
        HIST_ENTRY* entry = history_get(i);

        if (entry != nullptr) {
            std::println("{:5}  {}", i, entry->line);
        }
    }

    return 0;
}

int HistoryManager::read_builtin(const std::string& path) {
    auto lines = fs_utils::read_lines(path);

    for (const auto& line : lines) {
        add_history(line.c_str());
    }

    return 0;
}

int HistoryManager::write_builtin(const std::string& path) {
    auto lines = HistoryManager::collect_lines(history_base, history_base + history_length);

    bool write_result = fs_utils::write_str_vector_to_file(path, lines, WriteMode::Trunc);

    if (!write_result) {
        return 1;
    }

    append_offsets_[path] = history_length;

    return 0;
}

int HistoryManager::append_builtin(const std::string& path) {
    int offset = append_offsets_[path];

    int first = history_base + offset;
    int last_exclusive = history_base + history_length;

    auto lines = HistoryManager::collect_lines(first, last_exclusive);

    bool write_result = fs_utils::write_str_vector_to_file(path, lines, WriteMode::Append);

    if (!write_result) {
        return 1;
    }

    append_offsets_[path] = history_length;

    return 0;
}

std::vector<std::string> HistoryManager::collect_lines(int first, int last_exclusive) const {
    std::vector<std::string> lines;

    for (int i = first; i < last_exclusive; ++i) {
        HIST_ENTRY* entry = history_get(i);

        if (entry != nullptr && entry->line != nullptr) {
            lines.emplace_back(entry->line);
        }
    }

    return lines;
}

int HistoryManager::run(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        return HistoryManager::print_all();
    }

    if (args.size() == 2) {
        auto possible_limit = string_utils::to_int(args[1]);

        if (!possible_limit.has_value() || possible_limit.value() < 1) {
            std::println(stderr, "history: {}: positive numeric argument required", args[1]);
            return 2;
        }

        int count = possible_limit.value();
        return HistoryManager::print_last(count);
    }

    if (args.size() == 3) {
        const auto& flag = args[1];
        const auto& path = args[2];
        if (flag == "-r") {
            return HistoryManager::read_builtin(path);
        } else if (flag == "-w") {
            return HistoryManager::write_builtin(path);
        } else if (flag == "-a") {
            return HistoryManager::append_builtin(path);
        }
    }

    return 0;
}
