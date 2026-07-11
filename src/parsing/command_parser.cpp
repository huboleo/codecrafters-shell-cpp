#include "parsing/command_parser.hpp"
#include "redirection/redirection_types.hpp"
#include "utils/string_utils.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using command_parser::ParseError;

namespace {
bool is_variable_start(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    return c == '_' || std::isalpha(uc);
}

bool is_variable_body(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    return c == '_' || std::isalnum(uc);
}
std::string
expand_single_argument(const std::string& input,
                       const std::vector<std::pair<std::string, std::string>>& declared_variables) {
    std::string result;

    for (size_t i{0}; i < input.size();) {
        if (input[i] != '$') {
            result.push_back(input[i]);
            ++i;
            continue;
        }

        if (i + 1 >= input.size()) {
            result.push_back('$');
            ++i;
            continue;
        }

        std::string variable_name;
        size_t next_position = i + 1;

        if (input[i + 1] == '{') {
            size_t name_start = i + 2;
            size_t closing_brace = input.find('}', name_start);
            if (closing_brace == std::string::npos) {
                result.push_back('$');
                ++i;
                continue;
            }
            variable_name = input.substr(name_start, closing_brace - name_start);
            next_position = closing_brace + 1;
        } else {
            size_t name_start = i + 1;

            if (!is_variable_start(input[name_start])) {
                result.push_back('$');
                ++i;
                continue;
            }

            size_t name_end = name_start + 1;

            while (name_end < input.size() && is_variable_body(input[name_end])) {
                ++name_end;
            }

            variable_name = input.substr(name_start, name_end - name_start);
            next_position = name_end;
        }

        auto variable_it = std::find_if(
            declared_variables.begin(), declared_variables.end(),
            [&variable_name](const auto& pair) { return pair.first == variable_name; });

        if (variable_it != declared_variables.end()) {
            result += variable_it->second;
        }

        i = next_position;
    }

    return result;
}
} // namespace

std::vector<std::string> command_parser::split_command(const std::string& input) {
    std::vector<std::string> parts;
    std::string current;

    bool in_single_quotes = false;
    bool in_double_quotes = false;
    bool is_escape = false;

    for (char character : input) {

        if (is_escape) {
            current.push_back(character);
            is_escape = false;
            continue;
        }

        if (character == '\\' && !in_single_quotes) {
            is_escape = true;
            continue;
        }

        if (character == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
        } else if (character == '\"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
        } else if (std::isspace(static_cast<unsigned char>(character)) && !in_single_quotes &&
                   !in_double_quotes) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(character);
        }
    }

    if (!current.empty()) {
        parts.push_back(current);
    }

    return parts;
}

std::string_view command_parser::parse_error_message(ParseError error) {
    switch (error) {
    case ParseError::MissingRedirectionTarget:
        return "syntax error: expected file after redirection";
    case ParseError::UnexpectedBackgroundToken:
        return "syntax error near unexpected token '&'";
    case ParseError::EmptyPipelineStage:
        return "syntax error: empty pipeline stage";
    }

    return "syntax error";
}

command_parser::Command command_parser::parse_command(const std::vector<std::string>& parts) {
    command_parser::Command result;

    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts.at(i) == ">" || parts.at(i) == "1>") {
            if (i + 1 >= parts.size()) {
                result.parse_error = ParseError::MissingRedirectionTarget;
                return result;
            }

            result.stdout_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Truncate};
            ++i;
        } else if (parts.at(i) == ">>" || parts.at(i) == "1>>") {
            if (i + 1 >= parts.size()) {
                result.parse_error = ParseError::MissingRedirectionTarget;
                return result;
            }

            result.stdout_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Append};
            ++i;
        } else if (parts.at(i) == "2>") {
            if (i + 1 >= parts.size()) {
                result.parse_error = ParseError::MissingRedirectionTarget;
                return result;
            }

            result.stderr_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Truncate};
            ++i;
        } else if (parts.at(i) == "2>>") {
            if (i + 1 >= parts.size()) {
                result.parse_error = ParseError::MissingRedirectionTarget;
                return result;
            }

            result.stderr_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Append};
            ++i;
        } else if (parts.at(i) == "&") {
            result.parse_error = ParseError::UnexpectedBackgroundToken;
            return result;
        } else {
            result.args.push_back(parts[i]);
        }
    }

    return result;
}

command_parser::ParsedLine command_parser::parse_line(std::vector<std::string> parts) {
    ParsedLine result;

    if (!parts.empty() && parts.back() == "&") {
        result.should_run_in_background = true;
        parts.pop_back();
    }

    std::vector<std::string> current_stage;

    for (const auto& part : parts) {
        if (part == "|") {
            if (current_stage.empty()) {
                result.parse_error = ParseError::EmptyPipelineStage;
                return result;
            }

            auto command = parse_command(current_stage);
            if (command.parse_error.has_value()) {
                result.parse_error = command.parse_error;
                return result;
            }

            result.commands.push_back(command);
            current_stage.clear();
        } else {
            current_stage.push_back(part);
        }
    }

    if (current_stage.empty()) {
        result.parse_error = ParseError::EmptyPipelineStage;
        return result;
    }

    auto command = parse_command(current_stage);

    if (command.parse_error.has_value()) {
        result.parse_error = command.parse_error;
        return result;
    }

    result.commands.push_back(command);
    return result;
}

void command_parser::expand_variables(
    std::vector<std::string>& args,
    const std::vector<std::pair<std::string, std::string>>& declared_variables) {
    for (auto& arg : args) {
        arg = expand_single_argument(arg, declared_variables);
    }
}
