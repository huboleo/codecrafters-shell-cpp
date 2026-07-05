#include "output_redirect.hpp"
#include "redirection_types.hpp"
#include <string>
#include <utility>
#include <vector>

#include <cstdio>
#include <fcntl.h>
#include <print>
#include <unistd.h>

OutputRedirect::~OutputRedirect() { restore_all(); }

bool OutputRedirect::redirect_stdout(const std::string& path, RedirectMode mode) {
    return redirect_fd(STDOUT_FILENO, path, mode);
}

bool OutputRedirect::redirect_stderr(const std::string& path, RedirectMode mode) {
    return redirect_fd(STDERR_FILENO, path, mode);
}

bool OutputRedirect::redirect_fd(int target_fd, const std::string& path, RedirectMode mode) {
    switch (target_fd) {
    case STDOUT_FILENO:
        std::fflush(stdout);
        break;
    case STDERR_FILENO:
        std::fflush(stderr);
        break;
    default:
        break;
    }

    int saved_fd = dup(target_fd);

    if (saved_fd == -1) {
        return false;
    }

    int flags = O_WRONLY | O_CREAT;

    if (mode == RedirectMode::Append) {
        flags |= O_APPEND;
    } else if (mode == RedirectMode::Truncate) {
        flags |= O_TRUNC;
    }

    int file_fd = open(path.c_str(), flags, 0644);

    if (file_fd == -1) {
        close(saved_fd);
        return false;
    }

    if (dup2(file_fd, target_fd) == -1) {
        close(file_fd);
        close(saved_fd);
        return false;
    }

    close(file_fd);
    saved_fds_.push_back({target_fd, saved_fd});

    return true;
}

void OutputRedirect::restore_all() {
    std::fflush(stdout);
    std::fflush(stderr);

    for (auto it = saved_fds_.rbegin(); it != saved_fds_.rend(); ++it) {
        auto [target_fd, saved_fd] = *it;

        if (dup2(saved_fd, target_fd) == -1) {
            std::println(stderr, "failed to restore file descriptor");
        }

        close(saved_fd);
    }

    saved_fds_.clear();
}
