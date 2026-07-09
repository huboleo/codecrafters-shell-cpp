#pragma once
#include <string>
#include <sys/types.h>
#include <vector>

enum class JobStatus {
    Running,
    Done,
};

struct Job {
    int id;
    pid_t pid;
    std::string command;
    JobStatus status;
};

class JobTable {
  public:
    /// Returns an assigned background job id;
    int add(pid_t f, const std::string& command);
    void remove_done();
    void refresh();
    void print() const;

  private:
    std::vector<Job> background_jobs_;
    int next_job_id_ = 1;
};
