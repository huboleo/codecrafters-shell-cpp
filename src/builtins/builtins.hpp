#pragma once

#include "shell/shell_context.hpp"
#include <string>
#include <vector>

namespace builtins {
const std::vector<std::string>& names();
bool is_builtin(const std::string& command);
[[nodiscard]] int run(const std::vector<std::string>& args, ShellContext& shell_context);
} // namespace builtins
