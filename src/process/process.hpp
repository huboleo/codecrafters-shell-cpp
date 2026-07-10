#pragma once
#include "parsing/command_parser.hpp"
#include "shell/shell_context.hpp"
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

namespace process {
struct PipelineResult {
    int exit_status = 0;
    std::optional<pid_t> background_pid;
};

void run_executable(const std::string& path, const std::vector<std::string>& args);

std::optional<pid_t> run_background_job(const std::string& path,
                                        const std::vector<std::string>& args);

std::vector<std::string> run_completer_script(const std::string& path,
                                              const std::vector<std::string>& args,
                                              const std::string& comp_line, int comp_point);

PipelineResult run_pipeline(const std::vector<command_parser::Command>& commands,
                            ShellContext& shell_context, bool run_in_background);

}; // namespace process
