#pragma once

#include "jobs/job_table.hpp"
#include <string>
#include <utility>
#include <vector>

struct ShellContext {
    JobTable& job_table;
    std::vector<std::pair<std::string, std::string>>& registered_completions;
    bool should_exit = false;
};
