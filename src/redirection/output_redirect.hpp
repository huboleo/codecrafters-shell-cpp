#pragma once

#include "redirection/redirection_types.hpp"
#include <string>
#include <utility>
#include <vector>

class OutputRedirect {
  public:
    OutputRedirect() = default;
    ~OutputRedirect();
    OutputRedirect(const OutputRedirect&) = delete;
    OutputRedirect& operator=(const OutputRedirect&) = delete;
    [[nodiscard]] bool redirect_stdout(const std::string& path, RedirectMode mode);
    [[nodiscard]] bool redirect_stderr(const std::string& path, RedirectMode mode);

  private:
    std::vector<std::pair<int, int>> saved_fds_;
    bool redirect_fd(int target_fd, const std::string& path, RedirectMode mode);
    void restore_all();
};
