#pragma once
#include <string>
#include <vector>

namespace completion {
std::vector<std::string> get_command_candidates(const std::vector<std::string>& builtins);
}
