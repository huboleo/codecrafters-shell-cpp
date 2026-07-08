#pragma once
#include <vector>
#include <string>

namespace process {
  void run_executable(const std::string& path,
                    const std::vector<std::string>& args);

std::vector<std::string> run_and_capture_lines(const std::string& path);

};
