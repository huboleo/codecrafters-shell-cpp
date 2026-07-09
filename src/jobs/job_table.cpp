#include "job_table.hpp"

#include "utils/string_utils.hpp"
#include <algorithm>
#include <cerrno>
#include <print>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

namespace {
std::string_view to_string(JobStatus status) {
    switch (status) {
    case JobStatus::Running:
        return "Running";
    case JobStatus::Done:
        return "Done";
    }

    return "Unknown";
}
} // namespace

int JobTable::add(pid_t pid, const std::string& command) {
    if (background_jobs_.empty()) {
        next_job_id_ = 1;
    } else {
        auto it = std::max_element(
            background_jobs_.begin(), background_jobs_.end(),
            [](const auto& first, const auto& second) { return first.id < second.id; });
        if (it != background_jobs_.end()) {
            next_job_id_ = it->id + 1;
        }
    }

    auto current_id = next_job_id_;
    background_jobs_.push_back(Job{
        .id = current_id,
        .pid = pid,
        .command = command,
        .status = JobStatus::Running,
    });

    next_job_id_++;
    return current_id;
}

void JobTable::refresh() {
    for (auto& job : background_jobs_) {
        int status;
        auto result = waitpid(job.pid, &status, WNOHANG);
        if (result == job.pid) {
            job.status = JobStatus::Done;
            job.command = string_utils::rtrim(job.command);

            if (!job.command.empty() && job.command.back() == '&') {
                job.command.pop_back();
                job.command = string_utils::rtrim(job.command);
            }
        } else if (result == -1 && errno == ECHILD) {
            job.status = JobStatus::Done;
        }
    }
}

void JobTable::remove_done() {
    std::erase_if(background_jobs_, [](const auto& job) { return job.status == JobStatus::Done; });
}

void JobTable::print_all() const {
    for (size_t i = 0; i < background_jobs_.size(); ++i) {
        const auto& job = background_jobs_[i];

        char mark = ' ';

        if (i == background_jobs_.size() - 1) {
            mark = '+';
        } else if (i == background_jobs_.size() - 2) {
            mark = '-';
        }

        std::println("[{}]{}  {:<24}{}", job.id, mark, to_string(job.status), job.command);
    }
}

void JobTable::print_done() const {
    for (size_t i = 0; i < background_jobs_.size(); ++i) {
        const auto& job = background_jobs_[i];

        if (job.status != JobStatus::Done) {
            continue;
        }

        char mark = ' ';

        if (i == background_jobs_.size() - 1) {
            mark = '+';
        } else if (i == background_jobs_.size() - 2) {
            mark = '-';
        }

        std::println("[{}]{}  {:<24}{}", job.id, mark, to_string(job.status), job.command);
    }
}
