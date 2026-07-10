#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class HistoryManager {

  public:
    int run(const std::vector<std::string>& args);

    bool load_from_file(const std::string& path);
    bool write_to_file(const std::string& path);

    bool append_to_file(const std::string& path);

  private:
    std::unordered_map<std::string, int> append_offsets_;

    int print_all() const;
    int print_last(int count) const;
    int read_builtin(const std::string& path);
    int write_builtin(const std::string& path);
    int append_builtin(const std::string& path);

    std::vector<std::string> collect_lines(int first, int last_exclusive) const;
};
