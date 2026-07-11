#pragma once

#include "history/history_manager.hpp"
#include "jobs/job_table.hpp"
#include <string>
#include <utility>
#include <vector>

struct ShellContext {
    JobTable& job_table;
    HistoryManager& history_manager;
    std::vector<std::pair<std::string, std::string>>& registered_completions;
    std::vector<std::pair<std::string, std::string>>& stored_variables;
    bool should_exit = false;
};
