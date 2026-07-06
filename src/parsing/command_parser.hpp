#pragma once
#include "redirection/redirection_types.hpp"
#include <optional>
#include <string>
#include <vector>

namespace command_parser {
struct Command {
    std::vector<std::string> args;
    std::optional<RedirectTarget> stdout_redirect;
    std::optional<RedirectTarget> stderr_redirect;
};
std::vector<std::string> split_command(const std::string& input);
Command parse_command(const std::vector<std::string>& parts);
} // namespace command_parser
