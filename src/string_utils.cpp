#include "string_utils.hpp"
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

std::string string_utils::ltrim(const std::string& input) {
    auto start = input.find_first_not_of(" \t\n\r\f\v");

    if (start == std::string::npos) {
        return "";
    }

    return input.substr(start);
}

std::vector<std::string> string_utils::split_whitespace(const std::string& input) {
    std::stringstream ss(input);
    std::vector<std::string> parts;
    std::string word;

    while (ss >> word) {
        parts.push_back(word);
    }

    return parts;
}

std::vector<std::string> string_utils::split_command(const std::string& input) {
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
