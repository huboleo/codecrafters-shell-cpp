#pragma once
#include <string>
#include <vector>

namespace process {
void run_executable(const std::string& path, const std::vector<std::string>& args);

std::vector<std::string> run_completer_script(const std::string& path,
                                              const std::vector<std::string>& args);

} // namespace process
