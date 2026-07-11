#pragma once
#include "redirection/redirection_types.hpp"
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace command_parser {
enum class ParseError {
    MissingRedirectionTarget,
    UnexpectedBackgroundToken,
    EmptyPipelineStage,
};

struct Command {
    std::vector<std::string> args;
    std::optional<RedirectTarget> stdout_redirect;
    std::optional<RedirectTarget> stderr_redirect;
    std::optional<ParseError> parse_error;
};

struct ParsedLine {
    std::vector<Command> commands;
    bool should_run_in_background = false;
    std::optional<ParseError> parse_error;
};
std::vector<std::string> split_command(const std::string& input);
Command parse_command(const std::vector<std::string>& parts);
std::string_view parse_error_message(ParseError error);
ParsedLine parse_line(std::vector<std::string> parts);
void expand_variables(std::vector<std::string>& args,
                      const std::vector<std::pair<std::string, std::string>>& declared_variables);
} // namespace command_parser
