#include "command_parser.hpp"
#include "redirection_types.hpp"
#include <string>
#include <vector>

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

command_parser::Command command_parser::parse_command(const std::vector<std::string>& parts) {
    command_parser::Command result;

    for (size_t i = 0; i < parts.size(); ++i) {
        if ((parts.at(i) == ">" || parts.at(i) == "1>") && i + 1 < parts.size()) {
            result.stdout_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Truncate};
            ++i;
        } else if ((parts.at(i) == ">>" || parts.at(i) == "1>>") && i + 1 < parts.size()) {
            result.stdout_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Append};
            ++i;
        } else if (parts.at(i) == "2>" && i + 1 < parts.size()) {
            result.stderr_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Truncate};
            ++i;
        } else if (parts.at(i) == "2>>" && i + 1 < parts.size()) {
            result.stderr_redirect =
                RedirectTarget{.path = parts[i + 1], .mode = RedirectMode::Append};
            ++i;
        } else {
            result.args.push_back(parts[i]);
        }
    }

    return result;
}
